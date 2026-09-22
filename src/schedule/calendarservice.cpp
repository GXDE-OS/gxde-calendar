/*
 * Copyright (C) 2026 CharOfString <root@charofstring.cc>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ----------------------------------------------------------------------------
 * Origin copyright bearer: 2019 - 2026 UnionTech Software Technology Co., Ltd.
 * Method signatures follow the reference's DBus interface
 * com.deepin.dataserver.Calendar.Account (DDE calendar tag 6.6.0).  The
 * reference splits client and service into two processes talking over DBus;
 * gxde-calendar is a single process, so this class is the in-process
 * replacement for that boundary rather than a port of any single upstream file.
 * Upstream license: LGPL-3.0-or-later, relicensed under GPL-3.0-or-later as
 * permitted by LGPL-3.0 section 3.
 * ----------------------------------------------------------------------------
 * 方法签名对照 dde-calendar 的 com.deepin.dataserver.Calendar.Account 接口，
 * 但走进程内调用：参考实现是 client/service 双进程 + DBus，本项目是单进程。
 * 没有账户系统（无 CalDAV / UnionID / 登录），只有本地日历 + ICS。
 */

#include "calendarservice.h"

#include "commondef.h"
#include "ddatabase.h"
#include "icsmanager.h"
#include "scheduledatabase.h"
#include "units.h"

#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QSet>
#include <QTimer>
#include <QUrl>
#include <QUuid>

namespace {

// 订阅日历要建一个用户侧的日程类型，跟用户自建日历是同一套字段
DScheduleType::Ptr makeUserType(const QString &typeID, const QString &typeName,
                                const QString &displayName, const QString &colorID)
{
    DScheduleType::Ptr type(new DScheduleType);
    type->setTypeID(typeID);
    type->setTypeName(typeName);
    type->setDisplayName(displayName);
    type->setColorID(colorID);
    type->setPrivilege(DScheduleType::User);
    type->setShowState(DScheduleType::Show);
    type->setDtCreate(QDateTime::currentDateTime());
    return type;
}

// 订阅列表上的兜底名字。名字留空时 subscribeIcs() 拿整条地址当名字，列表里名字和
// 地址就成了同一串被省略的 URL，几个订阅摆一起分不清哪个是哪个。这里取「域名 / 文件名」，
// 短，而且正好是区分订阅的那部分（google 的私有日历文件名是随机串，同域名下也能区分）。
QString friendlySubscriptionName(const QString &url)
{
    const QUrl parsed(url);
    const QString host = parsed.host();
    if (host.isEmpty()) {
        return url;
    }

    const QString file = parsed.fileName();
    return file.isEmpty() ? host : QStringLiteral("%1 / %2").arg(host, file);
}

} // namespace

CalendarService *CalendarService::instance()
{
    static CalendarService *s_instance = new CalendarService;
    return s_instance;
}

CalendarService::CalendarService(QObject *parent)
    : QObject(parent)
    , m_db(new ScheduleDataBase(this))
    , m_ics(new IcsManager(this))
{
    m_db->setDBPath(getDBPath());
    // initDBData() 在库文件不存在时建库建表，并写入九色系统调色板和
    // 工作/生活/其他/节假日四个系统类型（外加本项目自建的默认日历）；
    // 库已存在时只开连接。种子数据归数据层管，这里不再另建一份。
    m_db->initDBData();

    connect(m_ics, &IcsManager::fetchFinished, this,
            [this](const QString &token, bool ok, bool notModified, const QByteArray &data,
                   const QString &etag, const QString &error) {
                const QString typeID = token;
                if (!ok) {
                    //失败次数决定下次多久再试，见 icsRefetchDue()
                    m_icsFailCount.insert(typeID, m_icsFailCount.value(typeID) + 1);
                    qCWarning(ServiceLogger) << "ICS subscription fetch failed:" << typeID << error;
                    emit icsRefreshFinished(typeID, false, error);
                    return;
                }
                //拉成功就回到正常间隔
                m_icsFailCount.remove(typeID);

                //304：远端没变，本地内容就是最新的，记一下时间即可
                if (notModified) {
                    markIcsSynced(typeID, QString());
                    emit icsRefreshFinished(typeID, true, QString());
                    return;
                }

                const DSchedule::List schedules = m_ics->loadFromData(data);
                if (schedules.isEmpty()) {
                    //拉取本身是成功的，只是这份日历里没有事件。本地已有的事件保持不动：
                    //远端返回空有可能是服务端临时抽风，直接清库会把数据丢掉
                    qCWarning(ServiceLogger) << "ICS subscription returned no events, type:" << typeID;
                    markIcsSynced(typeID, etag);
                    emit icsRefreshFinished(typeID, true, QString());
                    return;
                }

                // 整批替换：订阅是「远端说了算」的数据源，本地不留旧版本
                m_db->deleteSchedulesByScheduleTypeID(typeID, true);
                int imported = 0;
                for (const DSchedule::Ptr &schedule : schedules) {
                    schedule->setScheduleTypeID(typeID);
                    ensureUidAvailable(schedule);
                    if (!m_db->createSchedule(schedule).isEmpty()) {
                        ++imported;
                    }
                }

                markIcsSynced(typeID, etag);

                qCInfo(ServiceLogger) << "ICS subscription refreshed:" << typeID
                                      << "imported" << imported << "of" << schedules.size();
                emit scheduleUpdate();
                emit icsRefreshFinished(typeID, true, QString());
            });

    //订阅的自动刷新。数据层自己不跑定时器的话，订阅记录里的「刷新间隔」就只是个
    //摆设（refreshAllIcs() 没有任何调用方）。5 分钟查一次，真正到点的订阅才发请求。
    m_icsRefreshTimer = new QTimer(this);
    m_icsRefreshTimer->setInterval(5 * 60 * 1000);
    connect(m_icsRefreshTimer, &QTimer::timeout, this, [this] { refreshAllIcs(false); });
    m_icsRefreshTimer->start();
    //启动时补一次：上次同步可能是几天前（甚至从来没同步过）的事了
    QTimer::singleShot(5 * 1000, this, [this] { refreshAllIcs(false); });
}

void CalendarService::markIcsSynced(const QString &typeID, const QString &etag)
{
    ScheduleDataBase::IcsSubscription sub = m_db->getIcsSubscription(typeID);
    if (sub.typeID.isEmpty()) {
        return;
    }

    sub.lastSync = QDateTime::currentDateTime();
    //304 响应一般不带 ETag，这时候保留库里的那份
    if (!etag.isEmpty()) {
        sub.lastETag = etag;
    }
    m_db->upsertIcsSubscription(sub);
}

CalendarService::~CalendarService() = default;


void CalendarService::ensureUidAvailable(const DSchedule::Ptr &schedule)
{
    if (schedule.isNull()) {
        return;
    }
    // 主键取的是 schedulingID()，普通日程它就是 uid()
    if (!m_db->scheduleExistsByScheduleID(schedule->schedulingID())) {
        return;
    }

    const QString newUid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    // UID 和 schedulingID 一起换：只改 uid() 的话 schedulingID() 还指着旧值，
    // 主键照样冲突（两者要么相等，要么 schedulingID 为空）
    schedule->setSchedulingID(newUid, newUid);
    qCDebug(ServiceLogger) << "Imported event uid was already taken, reassigned to" << newUid;
}

///////////////日程

QString CalendarService::createSchedule(const DSchedule::Ptr &schedule)
{
    if (schedule.isNull()) {
        return QString();
    }
    const QString id = m_db->createSchedule(schedule);
    if (!id.isEmpty()) {
        emit scheduleUpdate();
    }
    return id;
}

bool CalendarService::updateSchedule(const DSchedule::Ptr &schedule)
{
    if (schedule.isNull()) {
        return false;
    }
    const bool ok = m_db->updateSchedule(schedule);
    if (ok) {
        emit scheduleUpdate();
    }
    return ok;
}

DSchedule::Ptr CalendarService::getScheduleByScheduleID(const QString &scheduleID)
{
    return m_db->getScheduleByScheduleID(scheduleID);
}

bool CalendarService::deleteScheduleByScheduleID(const QString &scheduleID)
{
    const bool ok = m_db->deleteScheduleByScheduleID(scheduleID, 1);
    if (ok) {
        emit scheduleUpdate();
    }
    return ok;
}

bool CalendarService::isSubscriptionCalendar(const QString &typeID)
{
    if (typeID.isEmpty()) {
        return false;
    }
    return !m_db->getIcsSubscription(typeID).typeID.isEmpty();
}

bool CalendarService::isReadOnlySchedule(const DSchedule::Ptr &schedule) const
{
    if (schedule.isNull() || schedule->scheduleTypeID().isEmpty()) {
        return true;
    }

    //类型都查不到（比如日程是迁移过来的残留数据）：改不了也删不掉
    if (m_db->getScheduleTypeByID(schedule->scheduleTypeID()).isNull()) {
        return true;
    }

    //唯一的只读来源是 ICS 订阅日历：内容属于远端，本地怎么改都会在下一次刷新
    //时被整批覆盖（构造函数里「整批替换」那段），删掉也会再拉回来
    return !m_db->getIcsSubscription(schedule->scheduleTypeID()).typeID.isEmpty();
}

bool CalendarService::isScheduleDeletable(const DSchedule::Ptr &schedule)
{
    return !isReadOnlySchedule(schedule);
}

bool CalendarService::isScheduleEditable(const DSchedule::Ptr &schedule)
{
    return !isReadOnlySchedule(schedule);
}

QMap<QDate, DSchedule::List> CalendarService::querySchedulesWithParameter(
        const DScheduleQueryPar::Ptr &queryPar)
{
    if (queryPar.isNull()) {
        return {};
    }

    DSchedule::List schedules;
    switch (queryPar->queryType()) {
    case DScheduleQueryPar::Query_RRule:
        schedules = m_db->querySchedulesByRRule(queryPar->key(), int(queryPar->rruleType()));
        break;
    case DScheduleQueryPar::Query_ScheduleID: {
        DSchedule::Ptr schedule = m_db->getScheduleByScheduleID(queryPar->key());
        if (!schedule.isNull()) {
            schedules.append(schedule);
        }
        break;
    }
    case DScheduleQueryPar::Query_Top:
    case DScheduleQueryPar::Query_None:
    default:
        schedules = m_db->querySchedulesByKey(queryPar->key());
        break;
    }

    // 展开重复日程、按天分组（含 Query_Top 的截断）
    return DSchedule::convertSchedules(queryPar, schedules);
}

///////////////日程类型

DScheduleType::List CalendarService::getScheduleTypeList()
{
    //对应参考实现 AccountItem::getScheduleTypeList()：privilege 为 None 的类型不往
    //界面上给，也就是 initSysType() 建的节假日类型。参考实现里它根本不是用户的日历：
    //那里的日程是服务端按农历现算的、从不落库，所以既不展示，也不许新建日程选它。
    //
    //这一层过滤不能省。类型表是按 rowid 顺序取的，节假日类型是 initSysType() 建的
    //第一个，漏掉过滤它就排在列表最前面：新建日程弹窗的日历下拉默认选中第一项，
    //用户建的日程会全落到节假日类型里去（颜色是节假日的橙，还删不掉）。
    DScheduleType::List list;
    const DScheduleType::List types = m_db->getScheduleTypeList();
    for (const DScheduleType::Ptr &type : types) {
        if (!type.isNull() && type->privilege() != DScheduleType::None) {
            list.append(type);
        }
    }
    return list;
}

DScheduleType::Ptr CalendarService::getScheduleTypeByID(const QString &typeID)
{
    return m_db->getScheduleTypeByID(typeID);
}

QString CalendarService::createScheduleType(const DScheduleType::Ptr &scheduleType)
{
    if (scheduleType.isNull()) {
        return QString();
    }
    const QString id = m_db->createScheduleType(scheduleType);
    if (!id.isEmpty()) {
        emit scheduleTypeUpdate();
    }
    return id;
}

bool CalendarService::updateScheduleType(const DScheduleType::Ptr &scheduleType)
{
    if (scheduleType.isNull()) {
        return false;
    }
    const bool ok = m_db->updateScheduleType(scheduleType);
    if (ok) {
        emit scheduleTypeUpdate();
    }
    return ok;
}

bool CalendarService::deleteScheduleTypeByID(const QString &typeID)
{
    if (typeID.isEmpty()) {
        return false;
    }
    // 订阅类型的删除要连订阅记录一起清掉，否则刷新定时器会一直找一个不存在的类型
    m_db->deleteIcsSubscription(typeID);
    m_db->deleteSchedulesByScheduleTypeID(typeID, 1);
    const bool ok = m_db->deleteScheduleTypeByID(typeID, 1);
    if (ok) {
        emit scheduleTypeUpdate();
        emit scheduleUpdate();
    }
    return ok;
}

///////////////类型颜色

DTypeColor::List CalendarService::getSysColors()
{
    return m_db->getSysColor();
}

QString CalendarService::getLocalTypeID()
{
    return m_db->getLocalTypeID();
}

QString CalendarService::getFestivalTypeID()
{
    return m_db->getFestivalTypeID();
}

QString CalendarService::resolveColorID(const QString &colorCode, bool allowCustomColor)
{
    const DTypeColor::List colors = getSysColors();

    if (!colorCode.isEmpty()) {
        //先按色值对调色板：九色都在表里，对上就能直接用
        for (const DTypeColor::Ptr &color : colors) {
            if (color->colorCode().compare(colorCode, Qt::CaseInsensitive) == 0) {
                return color->colorID();
            }
        }

        if (allowCustomColor) {
            //自定义色不在上面的调色板里（调色板只查 privilege=1 的内置色），
            //但如果已经有个日历用着这个色值，就复用它的那一行——
            //否则每次保存都往颜色表里插一行同样的颜色
            //（查全部类型而不是界面那一份：颜色有没有人用要看库里的实际情况，
            //  节假日类型也是占着一个颜色的）
            const DScheduleType::List types = m_db->getScheduleTypeList();
            for (const DScheduleType::Ptr &type : types) {
                if (type->getColorCode().compare(colorCode, Qt::CaseInsensitive) == 0
                    && !type->getColorID().isEmpty()) {
                    return type->getColorID();
                }
            }

            //色值表里没有，补一行，否则类型 join 不上颜色表，
            //这个日历在所有列表里都不会出现
            DTypeColor custom;
            custom.setColorCode(colorCode);
            custom.setPrivilege(DTypeColor::PriUser);
            if (m_db->addTypeColor(custom)) {
                //addTypeColor() 会把生成的 colorID 写回对象
                return custom.colorID();
            }
            qCWarning(ServiceLogger) << "Failed to add custom type color:" << colorCode;
        }
    }

    //没给颜色（或不让自定义）：挑一个还没有日历在用的颜色，免得新日历跟已有的撞色
    QSet<QString> usedColors;
    //同上：占用情况看全部类型，含不展示的节假日类型
    const DScheduleType::List types = m_db->getScheduleTypeList();
    for (const DScheduleType::Ptr &type : types) {
        usedColors.insert(type->getColorID());
    }
    for (const DTypeColor::Ptr &color : colors) {
        if (!usedColors.contains(color->colorID())) {
            return color->colorID();
        }
    }

    //九个颜色都被用掉了就用第一个，总得有颜色
    if (!colors.isEmpty()) {
        return colors.first()->colorID();
    }

    //调色板都没建起来（initDBData() 之后不该发生），至少给个能 join 上的旧值
    return DDataBase::GOtherColorID;
}

QString CalendarService::createUserScheduleType(const QString &name, const QString &preferredColorCode)
{
    if (name.isEmpty()) {
        return QString();
    }

    const DTypeColor::List colors = getSysColors();
    if (colors.isEmpty()) {
        //调色板都没建起来，建出来的类型会 join 不上颜色表，不如不建
        qCWarning(ServiceLogger) << "No system color available, cannot create type:" << name;
        return QString();
    }

    //文件里带的颜色只是提示，对不上调色板就另挑一个，不往颜色表里加别人写的色值
    const QString colorID = resolveColorID(preferredColorCode, false);

    const QString typeID = QUuid::createUuid().toString(QUuid::WithoutBraces);
    return createScheduleType(makeUserType(typeID, name, name, colorID));
}

///////////////ICS：本地文件

CalendarService::IcsFileHints CalendarService::readIcsFileHints(const QString &icsFilePath)
{
    IcsFileHints hints;

    const IcsManager::IcsFileInfo info = m_ics->readFileInfo(icsFilePath);
    if (!info.valid) {
        return hints;
    }

    hints.valid = true;
    hints.colorCode = info.colorCode;
    hints.eventCount = info.eventCount;
    //名字优先级跟参考实现一致：文件里的类型名 -> X-WR-CALNAME -> 文件名，
    //再截到 20 个字符，免得名字长到把列表撑变形
    hints.name = (info.typeName.isEmpty() ? QFileInfo(icsFilePath).baseName() : info.typeName).left(20);
    return hints;
}

bool CalendarService::importSchedule(const QString &icsFilePath, const QString &typeID,
                                     bool cleanExists)
{
    if (icsFilePath.isEmpty() || typeID.isEmpty()) {
        return false;
    }

    bool loaded = false;
    const DSchedule::List schedules = m_ics->loadFromFile(icsFilePath, &loaded);
    if (!loaded) {
        return false;
    }

    if (cleanExists) {
        m_db->deleteSchedulesByScheduleTypeID(typeID, true);
    }

    int imported = 0;
    for (const DSchedule::Ptr &schedule : schedules) {
        schedule->setScheduleTypeID(typeID);
        ensureUidAvailable(schedule);
        if (!m_db->createSchedule(schedule).isEmpty()) {
            ++imported;
        }
    }

    qCInfo(ServiceLogger) << "Imported" << imported << "of" << schedules.size()
                          << "events from" << icsFilePath << "into type" << typeID;

    if (imported > 0) {
        emit scheduleUpdate();
    }
    // 与参考实现一致：文件里没有事件也算成功
    return imported > 0 || schedules.isEmpty();
}

bool CalendarService::exportSchedule(const QString &icsFilePath, const QString &typeID)
{
    if (icsFilePath.isEmpty() || typeID.isEmpty()) {
        return false;
    }

    const DScheduleType::Ptr type = m_db->getScheduleTypeByID(typeID);
    if (type.isNull()) {
        qCWarning(ServiceLogger) << "Cannot export, unknown schedule type:" << typeID;
        return false;
    }

    DSchedule::List schedules;
    const QStringList ids = m_db->getScheduleIDListByTypeID(typeID);
    schedules.reserve(ids.size());
    for (const QString &id : ids) {
        DSchedule::Ptr schedule = m_db->getScheduleByScheduleID(id);
        if (!schedule.isNull()) {
            schedules.append(schedule);
        }
    }

    return m_ics->saveToFile(icsFilePath, schedules, typeID,
                             type->displayName(), type->getColorCode());
}

///////////////ICS：远程订阅

QString CalendarService::subscribeIcs(const QString &url, const QString &displayName,
                                      int refreshIntervalMin, const QString &colorCode)
{
    if (url.isEmpty()) {
        return QString();
    }

    const QString typeID = QUuid::createUuid().toString(QUuid::WithoutBraces);
    // 名字空着就用地址兜底，界面上至少能看出订阅的是什么
    const QString name = displayName.isEmpty() ? url : displayName;

    DScheduleType::Ptr type = makeUserType(typeID, name, name, resolveColorID(colorCode));
    // 订阅来的日历通常是只读的，用 Read 而不是 User
    type->setPrivilege(DScheduleType::Read);
    type->setDescription(url);
    if (m_db->createScheduleType(type).isEmpty()) {
        qCWarning(ServiceLogger) << "Failed to create type for ICS subscription:" << url;
        return QString();
    }

    ScheduleDataBase::IcsSubscription sub;
    sub.typeID = typeID;
    sub.url = url;
    sub.refreshIntervalMin = refreshIntervalMin;
    sub.dtCreate = QDateTime::currentDateTime();
    if (!m_db->upsertIcsSubscription(sub)) {
        // 订阅记录没写进去，类型已经建了，回滚掉免得留一个永远不刷新的空日历
        m_db->deleteScheduleTypeByID(typeID, 1);
        return QString();
    }

    qCInfo(ServiceLogger) << "Subscribed to ICS:" << url << "as type" << typeID;
    emit scheduleTypeUpdate();

    // 立刻拉一次，内容到了再发 scheduleUpdate
    m_ics->fetch(url, QString(), typeID);
    return typeID;
}

bool CalendarService::updateIcsSubscription(const QString &typeID, const QString &url,
                                            const QString &displayName, int refreshIntervalMin,
                                            const QString &colorCode)
{
    if (typeID.isEmpty() || url.isEmpty()) {
        return false;
    }

    ScheduleDataBase::IcsSubscription sub = m_db->getIcsSubscription(typeID);
    if (sub.typeID.isEmpty()) {
        qCWarning(ServiceLogger) << "No ICS subscription to update for type:" << typeID;
        return false;
    }

    const DScheduleType::Ptr type = getScheduleTypeByID(typeID);
    if (type.isNull()) {
        qCWarning(ServiceLogger) << "No schedule type to update for ICS subscription:" << typeID;
        return false;
    }

    //名字空着就用地址兜底，跟 subscribeIcs() 一致
    const QString name = displayName.isEmpty() ? url : displayName;
    type->setTypeName(name);
    type->setDisplayName(name);
    type->setColorID(resolveColorID(colorCode));
    type->setDescription(url);
    if (!m_db->updateScheduleType(type)) {
        qCWarning(ServiceLogger) << "Failed to update type for ICS subscription:" << typeID;
        return false;
    }

    //地址换了就把同步状态清掉：lastSync/lastETag 记的是旧地址的，留着的话新地址
    //要么等满一个刷新间隔才拉，要么拿旧 ETag 去问、被服务端当成「没变」而不返回内容
    const bool urlChanged = (sub.url != url);
    if (urlChanged) {
        sub.lastSync = QDateTime();
        sub.lastETag.clear();
    }
    sub.url = url;
    sub.refreshIntervalMin = refreshIntervalMin;
    if (!m_db->upsertIcsSubscription(sub)) {
        qCWarning(ServiceLogger) << "Failed to update ICS subscription:" << typeID;
        return false;
    }

    //改了地址/间隔/颜色之后重新开始算退避，不然刚失败过的订阅要等很久才试新地址
    m_icsLastAttempt.remove(typeID);
    m_icsFailCount.remove(typeID);

    qCInfo(ServiceLogger) << "Updated ICS subscription:" << typeID << url;
    emit scheduleTypeUpdate();

    //立刻拉一次，让用户马上看到改动的效果（地址没变时带上旧 ETag 做增量）
    m_ics->fetch(sub.url, sub.lastETag, typeID);
    return true;
}

bool CalendarService::unsubscribeIcs(const QString &typeID)
{
    if (typeID.isEmpty()) {
        return false;
    }

    const bool hadSubscription = m_db->deleteIcsSubscription(typeID);
    m_db->deleteSchedulesByScheduleTypeID(typeID, true);
    m_db->deleteScheduleTypeByID(typeID, 1);
    m_icsLastAttempt.remove(typeID);
    m_icsFailCount.remove(typeID);

    qCInfo(ServiceLogger) << "Unsubscribed ICS type:" << typeID;
    // 即使没有订阅记录也要发信号：类型和日程确实被删了
    emit scheduleUpdate();
    emit scheduleTypeUpdate();
    return hadSubscription;
}

bool CalendarService::refreshIcs(const QString &typeID)
{
    const ScheduleDataBase::IcsSubscription sub = m_db->getIcsSubscription(typeID);
    if (sub.typeID.isEmpty() || sub.url.isEmpty()) {
        qCWarning(ServiceLogger) << "No ICS subscription for type:" << typeID;
        return false;
    }
    //用户手动点的刷新：记一笔尝试时间，不然定时器下一轮还会再拉一次
    m_icsLastAttempt.insert(typeID, QDateTime::currentDateTime());
    m_ics->fetch(sub.url, sub.lastETag, typeID);
    return true;
}

bool CalendarService::icsRefetchDue(const QString &typeID, const QDateTime &lastSync,
                                    int refreshIntervalMin, const QDateTime &now) const
{
    //「上次成功同步」和「上次尝试」取更近的那个：失败不会更新 lastSync，
    //只看它的话失败的订阅会一直重发
    const QDateTime lastTry = m_icsLastAttempt.value(typeID);
    const QDateTime last = (lastSync.isValid() && lastSync > lastTry) ? lastSync : lastTry;
    if (!last.isValid()) {
        //从来没同步成功过也从没试过（刚订阅）：立刻拉
        return true;
    }

    const int fails = m_icsFailCount.value(typeID);
    //失败过就 5 分钟起步、每次翻倍（封顶 5 小时出头），但不超过用户设的间隔 ——
    //用户设 15 分钟就是 15 分钟，设 24 小时也不该因为一次超时等满一天
    const int waitMin = (fails > 0)
            ? qMin(5 << qMin(fails - 1, 6), qMax(refreshIntervalMin, 5))
            : refreshIntervalMin;

    return last.secsTo(now) >= waitMin * 60;
}

void CalendarService::refreshAllIcs(bool force)
{
    const QVector<ScheduleDataBase::IcsSubscription> subs = m_db->getIcsSubscriptionList();
    const QDateTime now = QDateTime::currentDateTime();
    for (const ScheduleDataBase::IcsSubscription &sub : subs) {
        //间隔为 0 是「不自动刷新」，自动刷新时直接跳过；手动刷新（force）不看间隔
        if (!force && sub.refreshIntervalMin <= 0) {
            continue;
        }
        if (!force && !icsRefetchDue(sub.typeID, sub.lastSync, sub.refreshIntervalMin, now)) {
            continue;
        }
        //手动刷新也记一笔，免得刚点完「全部刷新」定时器又拉一遍
        m_icsLastAttempt.insert(sub.typeID, now);
        m_ics->fetch(sub.url, sub.lastETag, sub.typeID);
    }
}

QVector<CalendarService::IcsSubscriptionInfo> CalendarService::getIcsSubscriptionList()
{
    QVector<IcsSubscriptionInfo> list;

    const QVector<ScheduleDataBase::IcsSubscription> subs = m_db->getIcsSubscriptionList();
    for (const ScheduleDataBase::IcsSubscription &sub : subs) {
        IcsSubscriptionInfo info;
        info.typeID = sub.typeID;
        info.url = sub.url;
        info.refreshIntervalMin = sub.refreshIntervalMin;
        info.lastSync = sub.lastSync;

        const DScheduleType::Ptr type = getScheduleTypeByID(sub.typeID);
        if (!type.isNull()) {
            info.displayName = type->displayName();
            info.colorCode = type->getColorCode();
        }
        //类型没了的孤儿订阅、或者名字当初就是拿地址兜的：给个短名，列表上能分清
        if (info.displayName.isEmpty() || info.displayName == sub.url) {
            info.displayName = friendlySubscriptionName(sub.url);
        }

        list.append(info);
    }

    return list;
}
