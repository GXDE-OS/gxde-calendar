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

#ifndef CALENDARSERVICE_H
#define CALENDARSERVICE_H

#include "dschedule.h"
#include "dschedulequerypar.h"
#include "dscheduletype.h"
#include "dtypecolor.h"

#include <QDateTime>
#include <QHash>
#include <QMap>
#include <QObject>
#include <QString>
#include <QVector>

class ScheduleDataBase;
class IcsManager;
class QTimer;

/**
 * @brief 日程数据层对外的唯一入口。
 *
 * 单例。首次访问时打开 ~/.config/GXDE/gxde-calendar/schedule.db，建库建表，
 * 种子数据（九色系统调色板、工作/生活/其他/节假日四个系统类型，以及默认的
 * 本地日历）由数据层的 ScheduleDataBase::initDBData() 写入，这里不再重复一份。
 *
 * 所有写操作完成后发 scheduleUpdate()/scheduleTypeUpdate()，与参考实现的
 * signalScheduleUpdate/signalScheduleTypeUpdate 对应，界面层据此刷新。
 */
class CalendarService : public QObject
{
    Q_OBJECT
public:
    static CalendarService *instance();

    ///////////////日程
    /**
     * @brief createSchedule   新建日程，返回日程 ID；失败返回空串
     */
    QString createSchedule(const DSchedule::Ptr &schedule);
    bool updateSchedule(const DSchedule::Ptr &schedule);
    DSchedule::Ptr getScheduleByScheduleID(const QString &scheduleID);
    bool deleteScheduleByScheduleID(const QString &scheduleID);

    /**
     * @brief querySchedulesWithParameter   按查询参数取日程并按日期分组
     *
     * 返回的 map 已经把重复日程展开到查询区间内的每一天（参考实现的
     * DSchedule::convertSchedules），界面层拿到就能直接按天画。
     */
    QMap<QDate, DSchedule::List> querySchedulesWithParameter(const DScheduleQueryPar::Ptr &queryPar);

    ///////////////日程类型
    DScheduleType::List getScheduleTypeList();
    DScheduleType::Ptr getScheduleTypeByID(const QString &typeID);
    QString createScheduleType(const DScheduleType::Ptr &scheduleType);
    bool updateScheduleType(const DScheduleType::Ptr &scheduleType);
    bool deleteScheduleTypeByID(const QString &typeID);

    /**
     * @brief getLocalTypeID      本项目的默认本地日历（initSysType() 建的 Local 类型）
     *
     * 新建日程时没有指定日历就用它。
     */
    QString getLocalTypeID();

    /**
     * @brief getFestivalTypeID   节假日类型
     */
    QString getFestivalTypeID();

    ///////////////类型颜色
    DTypeColor::List getSysColors();

    /**
     * @brief getRemindSchedule    取出所有设了提醒的日程（isAlarm = 1）
     *
     * 返回的日程是从库里的 ics 串重建的，因此带着各自的 alarm offset。
     * 提醒模块按它排定时器。
     */
    DSchedule::List getRemindSchedule();

    /**
     * @brief createUserScheduleType   新建一个「用户日历」（导入 .ics 时用）
     * @param preferredColorCode       首选颜色色值，为空或对不上系统调色板时自动挑
     *
     * 返回新建类型的 ID。颜色必须落到 typeColor 表里的某一行：类型表和颜色表是
     * inner join 的，颜色对不上这个日历在所有列表里都不会出现。
     */
    QString createUserScheduleType(const QString &name, const QString &preferredColorCode = QString());

    ///////////////ICS：本地文件
    /**
     * @brief IcsFileHints  导入弹窗预填用：文件里带的日历名和颜色
     */
    struct IcsFileHints {
        bool valid = false;   //文件存在且能解析
        QString name;         //推荐日历名（文件里的提示，退回文件名，最长 20 字符）
        QString colorCode;    //文件里带的颜色，可能是空
        int eventCount = 0;
    };

    /**
     * @brief readIcsFileHints   读取 .ics 文件里的类型提示（X-DDE-CALENDAR-TYPE-NAME
     *                           等），文件读不了时 valid 为 false
     */
    IcsFileHints readIcsFileHints(const QString &icsFilePath);

    /**
     * @brief importSchedule   把 .ics 文件导入到指定日程类型
     * @param cleanExists      导入前清空该类型已有日程（参考实现默认行为）；
     *                         导入到已有的日历要传 false，否则会把原有日程清掉
     */
    bool importSchedule(const QString &icsFilePath, const QString &typeID, bool cleanExists = true);

    /**
     * @brief exportSchedule   把指定日程类型导出成 .ics 文件
     */
    bool exportSchedule(const QString &icsFilePath, const QString &typeID);

    ///////////////ICS：远程订阅（参考实现没有，本项目新增）
    /**
     * @brief subscribeIcs     订阅一个远程 .ics 地址
     *
     * 会为这个订阅建一个日程类型（名字取 displayName），并立即拉取一次。
     * 返回新建类型的 ID；地址非法或建类型失败返回空串。
     * 拉取是异步的，成功入库后发 scheduleUpdate()。
     */
    QString subscribeIcs(const QString &url, const QString &displayName, int refreshIntervalMin);

    /**
     * @brief unsubscribeIcs   取消订阅：删类型、删该类型下的日程、删订阅记录
     */
    bool unsubscribeIcs(const QString &typeID);

    /**
     * @brief refreshIcs       重新拉取一个订阅（带 If-None-Match，未变更则不发信号）
     */
    bool refreshIcs(const QString &typeID);

    /**
     * @brief refreshAllIcs    拉取所有已到刷新时间的订阅
     * @param force            为 true 时忽略刷新间隔，全部拉一遍
     */
    void refreshAllIcs(bool force = false);

    /**
     * @brief 订阅列表里的一项，给管理界面用。
     *
     * 订阅记录（地址、间隔、上次同步）在 icsSubscription 表里，界面上还要显示名称
     * 和颜色，那两项挂在日程类型上，所以这里合成一份给界面。
     */
    struct IcsSubscriptionInfo {
        QString typeID;
        QString displayName;        //日程类型名，列表上的标题
        QString colorCode;          //类型颜色（hex），列表前的色点
        QString url;                //远程 .ics 地址
        int refreshIntervalMin = 0; //0 表示不自动刷新
        QDateTime lastSync;         //上次拉取成功的时间，无效表示还没同步过
    };

    /**
     * @brief getIcsSubscriptionList   当前所有 ICS 订阅，按创建时间排
     */
    QVector<IcsSubscriptionInfo> getIcsSubscriptionList();

signals:
    //日程数据有变化，界面需要重新查询
    void scheduleUpdate();
    //日程类型或颜色有变化
    void scheduleTypeUpdate();
    /**
     * @brief 一次订阅拉取结束。
     *
     * 成功后 lastSync 已经写库，界面重新取列表即可；ok 为 false 时 error 是失败
     * 原因（网络错误、HTTP 状态码等），可以直接显示给用户。「远端没变」（304）
     * 也算成功。
     */
    void icsRefreshFinished(const QString &typeID, bool ok, const QString &error);

private:
    explicit CalendarService(QObject *parent = nullptr);
    ~CalendarService() override;
    Q_DISABLE_COPY(CalendarService)

    /**
     * @brief ensureUidAvailable   导入前保证日程的 UID 在本库里没被占用
     *
     * schedules 表拿 UID 当主键。参考实现是每个账户一个库文件，UID 跨库不会撞；
     * 本项目所有日历共用一个库，同一份 .ics 导入第二个日历时全部事件都会因主键冲突
     * 被静默丢掉，所以撞了就换一个新 UID。
     */
    void ensureUidAvailable(const DSchedule::Ptr &schedule);

    /**
     * @brief markIcsSynced  记下这次拉取的时间和 ETag
     *
     * 内容有更新和「远端没变」（304）都要记：不记的话 refreshAllIcs() 会按间隔
     * 一遍遍重拉同一个地址。
     */
    void markIcsSynced(const QString &typeID, const QString &etag);

    /**
     * @brief icsRefetchDue   这个订阅现在到点了吗
     *
     * 拉取失败不会更新 lastSync（那是「上次成功同步」的时间），所以光看 lastSync 的话，
     * 从来没成功过的订阅每次 tick 都会重发请求 —— 5 分钟一次、一天 288 次，正是被
     * 服务端当成「automated queries」限流的原因。所以还要看上次「尝试」的时间，
     * 失败过的按 5 分钟起步逐次翻倍退避。
     */
    bool icsRefetchDue(const QString &typeID, const QDateTime &lastSync,
                       int refreshIntervalMin, const QDateTime &now) const;

    ScheduleDataBase *m_db = nullptr;
    IcsManager *m_ics = nullptr;
    //订阅的自动刷新定时器，见构造函数里的说明
    QTimer *m_icsRefreshTimer = nullptr;
    //上次发起拉取的时间 / 连续失败次数，见 icsRefetchDue()
    QHash<QString, QDateTime> m_icsLastAttempt;
    QHash<QString, int> m_icsFailCount;
};

#endif // CALENDARSERVICE_H
