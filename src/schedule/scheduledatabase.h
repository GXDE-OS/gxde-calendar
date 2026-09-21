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
 * Ported from DDE calendar tag 6.6.0 (upstream license: LGPL-3.0-or-later,
 * relicensed under GPL-3.0-or-later as permitted by LGPL-3.0 section 3).
 * Derived from DAccountDataBase: account, CalDAV, remind and upload-task members removed.
 * ----------------------------------------------------------------------------
 * 移植自 dde-calendar（src/calendar-service/src/dbmanager/daccountdatabase.h）。
 */
#ifndef SCHEDULEDATABASE_H
#define SCHEDULEDATABASE_H

#include "ddatabase.h"
#include "dschedule.h"
#include "dscheduletype.h"
#include "dtypecolor.h"

#include <QSharedPointer>
#include <QVector>
#include <QDateTime>

/*
 * ICS 订阅表（icsSubscription）：每条记录为一个远程 ICS 订阅。
 * 一个订阅对应一个日程类型（typeID 为主键，取值来自 scheduleType.typeID），
 * 存放远程地址、刷新间隔、上次成功拉取时间，以及上次响应的 ETag
 * （用于 If-None-Match 做增量拉取）。本地文件导入不写这张表，只有 URL 订阅才写。
 */

/*
 * 由参考实现的 DAccountDataBase 裁剪而来：本项目是单进程本地日历，
 * 没有账户体系，也没有 CalDAV / 云同步、提醒任务（remindTask）与
 * 上传任务（uploadTask）子系统，只保留本地日程、日程类型、类型颜色，
 * 并新增上面的 ICS 订阅表。
 */
class ScheduleDataBase : public DDataBase
{
    Q_OBJECT
public:
    typedef QSharedPointer<ScheduleDataBase> Ptr;

    explicit ScheduleDataBase(QObject *parent = nullptr);
    //初始化数据库数据，会创建数据库文件和相关数据表
    void initDBData() override;
    ///////////////日程信息
    //创建日程
    QString createSchedule(const DSchedule::Ptr &schedule);
    bool updateSchedule(const DSchedule::Ptr &schedule);
    //根据日程id获取日程信息
    DSchedule::Ptr getScheduleByScheduleID(const QString &scheduleID);
    bool scheduleExistsByScheduleID(const QString &scheduleID) const;
    bool isScheduleDeletedByScheduleID(const QString &scheduleID) const;

    //根据日程类型ID获取日程id列表
    QStringList getScheduleIDListByTypeID(const QString &typeID);
    DSchedule::List getScheduleListByTypeID(const QString &typeID);
    bool deleteScheduleByScheduleID(const QString &scheduleID, const int isDeleted = 0);
    bool deleteSchedulesByScheduleTypeID(const QString &typeID, const int isDeleted = 0);
    bool restoreSchedulesByScheduleTypeID(const QString &typeID);
    //根据关键字查询一定范围内的日程
    DSchedule::List querySchedulesByKey(const QString &key);
    //根据重复规则查询一定范围内的日程
    DSchedule::List querySchedulesByRRule(const QString &key, const int &rruleType);
    //获取需要提醒的日程信息
    DSchedule::List getRemindSchedule();

    ///////////////类型信息
    /**
     * @brief createScheduleType        创建日程类型
     * @param typeInfo
     * @return
     */
    QString createScheduleType(const DScheduleType::Ptr &scheduleType);
    virtual DScheduleType::Ptr getScheduleTypeByID(const QString &typeID, const int isDeleted = 0);
    virtual DScheduleType::List getScheduleTypeList(const int isDeleted = 0);
    bool scheduleTypeByUsed(const QString &typeID, const int isDeleted = 0);
    bool deleteScheduleTypeByID(const QString &typeID, const int isDeleted = 0);
    bool restoreScheduleTypeByID(const QString &typeID);
    bool updateScheduleType(const DScheduleType::Ptr &scheduleType);

    //获取节假日类型ID
    QString getFestivalTypeID();
    //获取默认本地日历的类型ID（本项目没有帐户体系，新建日程默认写到这里）
    QString getLocalTypeID();

    ///////////////////类型颜色
    bool addTypeColor(const DTypeColor::Ptr &typeColor);
    bool addTypeColor(DTypeColor &typeColor);
    void deleteTypeColor(const QString &colorNo);
    //获取内置类型颜色
    DTypeColor::List getSysColor();

    ///////////////ICS 订阅
    // 订阅的远程 ICS 地址。本地文件导入不写这张表，只有 URL 订阅才写。
    struct IcsSubscription {
        QString typeID;             // 对应 scheduleType.typeID，一个订阅就是一个日程类型
        QString url;                // 远程 .ics 地址
        int refreshIntervalMin = 0; // 刷新间隔（分钟），0 表示不自动刷新
        QDateTime lastSync;         // 上次成功拉取的时间
        QString lastETag;           // 上次响应的 ETag，用于 If-None-Match，为空表示服务端没给
        QDateTime dtCreate;
    };
    bool upsertIcsSubscription(const IcsSubscription &sub);
    IcsSubscription getIcsSubscription(const QString &typeID) const;
    QVector<IcsSubscription> getIcsSubscriptionList() const;
    bool deleteIcsSubscription(const QString &typeID);

protected:
    virtual void initScheduleType();
    //初始化系统类型
    virtual void initSysType();
    void systemTypeTran(const DScheduleType::Ptr &type);

protected:
    void createDB() override;
    //初始化日程数据库
    void initScheduleDB();
    void initTypeColor();
};

#endif // SCHEDULEDATABASE_H
