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
                Q_UNUSED(error)
                if (!ok || notModified) {
                    return;
                }

                const QString typeID = token;
                const DSchedule::List schedules = m_ics->loadFromData(data);
                if (schedules.isEmpty()) {
                    qCWarning(ServiceLogger) << "ICS subscription returned no events, type:" << typeID;
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

                ScheduleDataBase::IcsSubscription sub = m_db->getIcsSubscription(typeID);
                if (!sub.typeID.isEmpty()) {
                    sub.lastSync = QDateTime::currentDateTime();
                    if (!etag.isEmpty()) {
                        sub.lastETag = etag;
                    }
                    m_db->upsertIcsSubscription(sub);
                }

                qCInfo(ServiceLogger) << "ICS subscription refreshed:" << typeID
                                      << "imported" << imported << "of" << schedules.size();
                emit scheduleUpdate();
            });
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
    return m_db->getScheduleTypeList();
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

DSchedule::List CalendarService::getRemindSchedule()
{
    return m_db->getRemindSchedule();
}

///////////////ICS：本地文件

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
                                      int refreshIntervalMin)
{
    if (url.isEmpty()) {
        return QString();
    }

    const QString typeID = QUuid::createUuid().toString(QUuid::WithoutBraces);
    // 名字空着就用地址兜底，界面上至少能看出订阅的是什么
    const QString name = displayName.isEmpty() ? url : displayName;

    DScheduleType::Ptr type = makeUserType(typeID, name, name, DDataBase::GOtherColorID);
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

bool CalendarService::unsubscribeIcs(const QString &typeID)
{
    if (typeID.isEmpty()) {
        return false;
    }

    const bool hadSubscription = m_db->deleteIcsSubscription(typeID);
    m_db->deleteSchedulesByScheduleTypeID(typeID, true);
    m_db->deleteScheduleTypeByID(typeID, 1);

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
    m_ics->fetch(sub.url, sub.lastETag, typeID);
    return true;
}

void CalendarService::refreshAllIcs(bool force)
{
    const QVector<ScheduleDataBase::IcsSubscription> subs = m_db->getIcsSubscriptionList();
    const QDateTime now = QDateTime::currentDateTime();
    for (const ScheduleDataBase::IcsSubscription &sub : subs) {
        if (!force && sub.refreshIntervalMin > 0 && sub.lastSync.isValid()) {
            if (sub.lastSync.secsTo(now) < sub.refreshIntervalMin * 60) {
                continue;
            }
        }
        m_ics->fetch(sub.url, sub.lastETag, sub.typeID);
    }
}
