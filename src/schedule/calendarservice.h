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

#include <QMap>
#include <QObject>
#include <QString>

class ScheduleDataBase;
class IcsManager;

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

    ///////////////ICS：本地文件
    /**
     * @brief importSchedule   把 .ics 文件导入到指定日程类型
     * @param cleanExists      导入前清空该类型已有日程（参考实现默认行为）
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

signals:
    //日程数据有变化，界面需要重新查询
    void scheduleUpdate();
    //日程类型或颜色有变化
    void scheduleTypeUpdate();

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

    ScheduleDataBase *m_db = nullptr;
    IcsManager *m_ics = nullptr;
};

#endif // CALENDARSERVICE_H
