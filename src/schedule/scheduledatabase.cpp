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
 * 移植自 dde-calendar（src/calendar-service/src/dbmanager/daccountdatabase.cpp）。
 */
#include "scheduledatabase.h"

#include "commondef.h"
#include "units.h"
#include "pinyin/pinyinsearch.h"

#include <QSqlQuery>
#include <QString>
#include <QVariant>
#include <QDebug>
#include <QSqlError>
#include <QFile>

//ICS 订阅表：参考实现里没有这张表，是本项目新增的。
static const QString sql_create_icsSubscription =
    " CREATE TABLE if not exists icsSubscription (        "
    " typeID TEXT not null PRIMARY KEY,                   "
    " url TEXT not null,                                  "
    " refreshIntervalMin INTEGER not null,                "
    " lastSync DATETIME,                                  "
    " lastETag TEXT,                                      "
    " dtCreate DATETIME not null)";

ScheduleDataBase::ScheduleDataBase(QObject *parent)
    : DDataBase(parent)
{
    // 参考实现用 accountID 当连接名，因为每个账户一个库文件。
    // 本项目只有一个库，连接名固定。
    setConnectionName(QStringLiteral("gxde-calendar-schedule"));
}

QString ScheduleDataBase::createSchedule(const DSchedule::Ptr &schedule)
{
    if (!schedule.isNull()) {
        qCDebug(ServiceLogger) << "Creating schedule:" << schedule->summary() 
                              << "Start:" << schedule->dtStart().toString();
        SqliteQuery query(m_database);
        if (schedule->uid().isEmpty() || schedule->uid() == QStringLiteral("0")) {
            schedule->setUid(DDataBase::createUuid());
        }

        QString strSql("INSERT INTO schedules                                                   \
                       (scheduleID, scheduleTypeID, summary, description, allDay, dtStart   \
                       , dtEnd, isAlarm, titlePinyin,isLunar, ics, fileName, dtCreate, isDeleted)   \
                       VALUES(?, ?, ?, ?, ?, ?, ?, ?,?, ?, ?, ?, ?, ?);");
        if (query.prepare(strSql)) {
            query.addBindValue(schedule->schedulingID());
            query.addBindValue(schedule->scheduleTypeID());
            query.addBindValue(schedule->summary());
            query.addBindValue(schedule->description());
            query.addBindValue(schedule->allDay());
            query.addBindValue(dtToString(schedule->dtStart()));
            query.addBindValue(dtToString(schedule->dtEnd()));
            query.addBindValue(schedule->hasEnabledAlarms());
            query.addBindValue(pinyinsearch::getPinPinSearch()->CreatePinyin(schedule->summary()));
            query.addBindValue(schedule->lunnar());
            query.addBindValue(DSchedule::toIcsString(schedule));
            query.addBindValue(schedule->fileName());
            query.addBindValue(dtToString(schedule->created()));
            query.addBindValue(0);
            if (!query.exec()) {
                schedule->setUid("");
                qCWarning(ServiceLogger) << "Failed to create schedule:" << query.lastError().text();
            } else {
                qCDebug(ServiceLogger) << "Successfully created schedule with ID:" << schedule->uid();
            }
            if (query.isActive()) {
                query.finish();
            }
        } else {
            schedule->setUid("");
            qCWarning(ServiceLogger) << "Failed to prepare schedule creation query:" << query.lastError().text();
        }
    } else {
        qCWarning(ServiceLogger) << "Attempted to create null schedule";
    }
    return schedule->uid();
}

bool ScheduleDataBase::updateSchedule(const DSchedule::Ptr &schedule)
{
    bool resbool = false;
    if (!schedule.isNull()) {
        qCDebug(ServiceLogger) << "Updating schedule:" << schedule->summary() 
                              << "ID:" << schedule->schedulingID();
        SqliteQuery query(m_database);
        QString strSql("UPDATE schedules                                                  \
                       SET scheduleTypeID=?, summary=?, description=?, allDay=?           \
                , dtStart=?, dtEnd=?, isAlarm=?,titlePinyin=?, isLunar=?, ics=?, fileName=?             \
                , dtUpdate=?, dtDelete=?, isDeleted=0 WHERE scheduleID= ?;");
        if (query.prepare(strSql)) {
            query.addBindValue(schedule->scheduleTypeID());
            query.addBindValue(schedule->summary());
            query.addBindValue(schedule->description());
            query.addBindValue(schedule->allDay());
            query.addBindValue(dtToString(schedule->dtStart()));
            query.addBindValue(dtToString(schedule->dtEnd()));
            query.addBindValue(schedule->hasEnabledAlarms());
            query.addBindValue(pinyinsearch::getPinPinSearch()->CreatePinyin(schedule->summary()));
            query.addBindValue(schedule->lunnar());
            query.addBindValue(DSchedule::toIcsString(schedule));
            query.addBindValue(schedule->fileName());
            query.addBindValue(dtToString(schedule->lastModified()));
            query.addBindValue(QString());
            query.addBindValue(schedule->schedulingID());
            if (query.exec()) {
                resbool = true;
                qCDebug(ServiceLogger) << "Successfully updated schedule:" << schedule->schedulingID();
            } else {
                qCWarning(ServiceLogger) << "Failed to update schedule:" << query.lastError().text();
            }
            if (query.isActive()) {
                query.finish();
            }
        } else {
            qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
        }
    }
    return resbool;
}

DSchedule::Ptr ScheduleDataBase::getScheduleByScheduleID(const QString &scheduleID)
{
    QString strSql("SELECT  scheduleID, scheduleTypeID, summary, description, allDay, dtStart, dtEnd,   \
                   isAlarm,titlePinyin,isLunar, ics, fileName, dtCreate, dtUpdate, dtDelete, isDeleted  \
                   FROM schedules WHERE  scheduleID  = ? ;");
    SqliteQuery query(m_database);
    DSchedule::Ptr schedule;
    if (query.prepare(strSql)) {
        query.addBindValue(scheduleID);
        if (query.exec()) {
            if (query.next()) {
                const QString ics = query.value("ics").toString();
                DSchedule::Ptr parsedSchedule;
                if (DSchedule::fromIcsString(parsedSchedule, ics) && !parsedSchedule.isNull()) {
                    parsedSchedule->setScheduleTypeID(query.value("scheduleTypeID").toString());
                    schedule = parsedSchedule;
                } else {
                    qCWarning(ServiceLogger) << "Failed to parse schedule ICS for scheduleID:" << scheduleID;
                }
            }
        } else {
            qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
            return schedule;
        }
        if (query.isActive()) {
            query.finish();
        }
    } else {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
        return schedule;
    }

    return schedule;
}

bool ScheduleDataBase::scheduleExistsByScheduleID(const QString &scheduleID) const
{
    if (scheduleID.isEmpty()) {
        return false;
    }

    SqliteQuery query(m_database);
    if (!query.prepare(QStringLiteral(
            "SELECT 1 FROM schedules WHERE scheduleID = ? LIMIT 1"))) {
        return false;
    }
    query.addBindValue(scheduleID);
    return query.exec() && query.next();
}

bool ScheduleDataBase::isScheduleDeletedByScheduleID(const QString &scheduleID) const
{
    if (scheduleID.isEmpty()) {
        return false;
    }
    SqliteQuery query(m_database);
    if (!query.prepare(QStringLiteral(
            "SELECT isDeleted FROM schedules WHERE scheduleID = ? LIMIT 1"))) {
        return false;
    }
    query.addBindValue(scheduleID);
    return query.exec() && query.next() && query.value(0).toInt() != 0;
}

QStringList ScheduleDataBase::getScheduleIDListByTypeID(const QString &typeID)
{
    QStringList scheduleIDList;
    QString strSql("SELECT scheduleID FROM schedules WHERE  scheduleTypeID  = ?;");
    SqliteQuery query(m_database);
    if (query.prepare(strSql)) {
        query.addBindValue(typeID);
        if (query.exec()) {
            while (query.next()) {
                scheduleIDList.append(query.value("scheduleID").toString());
            }
        } else {
            qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
        }
        if (query.isActive()) {
            query.finish();
        }
    } else {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
    }

    return scheduleIDList;
}

bool ScheduleDataBase::deleteScheduleByScheduleID(const QString &scheduleID, const int isDeleted)
{
    QString strSql;
    if (isDeleted) {
        strSql = "DELETE FROM schedules WHERE scheduleID=?;";
    } else {
        strSql = QString("UPDATE schedules SET dtDelete = '%1' , isDeleted = 1  WHERE scheduleID=?").arg(dtToString(QDateTime::currentDateTime()));
    }
    SqliteQuery query(m_database);
    bool resBool = false;
    if (query.prepare(strSql)) {
        query.addBindValue(scheduleID);
        resBool = query.exec();
    }
    if (!resBool) {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }

    return resBool;
}

bool ScheduleDataBase::deleteSchedulesByScheduleTypeID(const QString &typeID, const int isDeleted)
{
    QString strSql;
    if (isDeleted) {
        strSql = "DELETE FROM schedules WHERE scheduleTypeID=?;";
    } else {
        strSql = QString("UPDATE schedules SET dtDelete = '%1' , isDeleted = 1  WHERE scheduleTypeID=?").arg(dtToString(QDateTime::currentDateTime()));
    }
    SqliteQuery query(m_database);
    bool resBool = false;
    if (query.prepare(strSql)) {
        query.addBindValue(typeID);
        resBool = query.exec();
    }

    if (!resBool) {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
    return resBool;
}

bool ScheduleDataBase::restoreSchedulesByScheduleTypeID(const QString &typeID)
{
    if (typeID.isEmpty()) {
        return false;
    }
    SqliteQuery query(m_database);
    if (!query.prepare(QStringLiteral(
            "UPDATE schedules SET dtDelete = NULL, isDeleted = 0 WHERE scheduleTypeID = ?"))) {
        return false;
    }
    query.addBindValue(typeID);
    return query.exec();
}

DSchedule::List ScheduleDataBase::getScheduleListByTypeID(const QString &typeID)
{
    DSchedule::List scheduleList;
    const QString sql = QStringLiteral(
        "SELECT ics, scheduleTypeID FROM schedules WHERE scheduleTypeID = ? AND isDeleted = 0");
    SqliteQuery query(m_database);
    if (query.prepare(sql)) {
        query.addBindValue(typeID);
        if (query.exec()) {
            while (query.next()) {
                DSchedule::Ptr schedule;
                if (DSchedule::fromIcsString(schedule, query.value("ics").toString())
                    && !schedule.isNull()) {
                    schedule->setScheduleTypeID(query.value("scheduleTypeID").toString());
                    scheduleList.append(schedule);
                }
            }
        } else {
            qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
    }
    if (query.isActive()) {
        query.finish();
    }
    return scheduleList;
}

DSchedule::List ScheduleDataBase::querySchedulesByKey(const QString &key)
{
    DSchedule::List scheduleList;
    QString strSql("SELECT s.scheduleID, s.scheduleTypeID, s.summary, s.description, s.allDay, s.dtStart, s.dtEnd,   \
             s.isAlarm,s.titlePinyin,s.isLunar, s.ics, s.fileName, s.dtCreate, s.dtUpdate, s.dtDelete, s.isDeleted      \
               FROM  schedules s  inner join scheduleType st WHERE s.isDeleted = 0 and st.showState =1 and s.scheduleTypeID  = st.typeID ");
    //如果关键字不为空，添加查询条件
    pinyinsearch *psearch = pinyinsearch::getPinPinSearch();
    QMap<QString, QString> sqlBindValue;
    QString strKey = key.trimmed();
    if (psearch->CanQueryByPinyin(key)) {
        //可以按照拼音查询
        QString pinyin = psearch->CreatePinyinQuery(strKey.toLower());
        strSql += QString("and ( instr(UPPER(s.summary), UPPER(:key)) OR s.titlePinyin LIKE :pinyin )");
        sqlBindValue[":key"] = key;
        sqlBindValue[":pinyin"] = pinyin;
    } else if (!key.isEmpty()) {
        //按照key查询
        strSql += QString(" and instr(UPPER(s.summary), UPPER(:key))");
        sqlBindValue[":key"] = key;
    }

    strSql.append(QString(" order by :strsort "));
    sqlBindValue[":strsort"] = "s.dtStart asc";

    SqliteQuery query(m_database);
    if (query.prepare(strSql)) {
        for (auto iter = sqlBindValue.constBegin(); iter != sqlBindValue.constEnd(); iter++) {
            query.bindValue(iter.key(), iter.value());
        }

        if (query.exec()) {
            while (query.next()) {
                DSchedule::Ptr schedule;
                QString &&icsStr = query.value("ics").toString();
                // 同 getScheduleByScheduleID()：解析失败时 schedule 仍是空指针，
                // 直接往下用就是空指针解引用，这里整行跳过。
                if (!DSchedule::fromIcsString(schedule, icsStr) || schedule.isNull()) {
                    qCWarning(ServiceLogger) << "Failed to parse schedule ICS:"
                                             << query.value("scheduleID").toString();
                    continue;
                }
                schedule->setScheduleTypeID(query.value("scheduleTypeID").toString());
                scheduleList.append(schedule);
            }
        } else {
            qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
    return scheduleList;
}

DSchedule::List ScheduleDataBase::querySchedulesByRRule(const QString &key, const int &rruleType)
{
    DSchedule::List scheduleList;
    QString strSql("SELECT  scheduleID, scheduleTypeID, summary, description, allDay, dtStart, dtEnd,   \
                   isAlarm,titlePinyin,isLunar, ics, fileName, dtCreate, dtUpdate, dtDelete, isDeleted  \
                   FROM schedules ");
    SqliteQuery query(m_database);
    if (!key.isEmpty()) {
        strSql += " WHERE  summary  = ? ";
    }
    if (query.prepare(strSql)) {
        if (!key.isEmpty()) {
            query.addBindValue(key);
        }

        if (query.exec()) {
            while (query.next()) {
                DSchedule::Ptr schedule = DSchedule::Ptr(new DSchedule);
                QString &&icsStr = query.value("ics").toString();
                DSchedule::fromIcsString(schedule, icsStr);
                schedule->setScheduleTypeID(query.value("scheduleTypeID").toString());
                DSchedule::RRuleType rRuleType = schedule->getRRuleType();
                //如果存在重复规则
                if (schedule->recurs()) {
                    //如果为需要获取的重复规则
                    if (rruleType == rRuleType) {
                        scheduleList.append(schedule);
                    }
                }
            }
        } else {
            qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
    return scheduleList;
}

void ScheduleDataBase::initDBData()
{
    //如果不存在对应的数据库则创建
    if (!dbFileExists()) {
        qCDebug(ServiceLogger) << "Database file does not exist, creating new database";
        createDB();
        initTypeColor();
        initScheduleDB();
        initScheduleType();
    } else {
        //如果存在则连接数据库
        qCDebug(ServiceLogger) << "Database file exists, opening connection";
        dbOpen();
    }
}

QString ScheduleDataBase::createScheduleType(const DScheduleType::Ptr &scheduleType)
{
    qCDebug(ServiceLogger) << "Creating schedule type:" << scheduleType->typeName();
    QString strSql("INSERT INTO scheduleType (                      \
                   typeID, typeName, typeDisplayName, typePath,     \
                   typeColorID, description, privilege, showState,  \
                   syncTag,dtCreate,isDeleted)                      \
               VALUES(?,?,?,?,?,?,?,?,?,?,?)");
    SqliteQuery query(m_database);
    if (query.prepare(strSql)) {
        if (scheduleType->typeID().size() < 30) {
            scheduleType->setTypeID(DDataBase::createUuid());
            qCDebug(ServiceLogger) << "Generated new type ID:" << scheduleType->typeID();
        }

        query.addBindValue(scheduleType->typeID());
        query.addBindValue(scheduleType->typeName());
        query.addBindValue(scheduleType->displayName());
        query.addBindValue(scheduleType->typePath());
        query.addBindValue(scheduleType->typeColor().colorID());
        query.addBindValue(scheduleType->description());
        query.addBindValue(int(scheduleType->privilege()));
        query.addBindValue(scheduleType->showState());
        query.addBindValue(scheduleType->syncTag());
        query.addBindValue(dtToString(scheduleType->dtCreate()));
        query.addBindValue(scheduleType->deleted());

        if (!query.exec()) {
            qCWarning(ServiceLogger) << "Failed to create schedule type:" << query.lastError().text();
            scheduleType->setTypeID("");
        }
    } else {
        qCWarning(ServiceLogger) << "Failed to prepare schedule type creation query:" << query.lastError().text();
        scheduleType->setTypeID("");
    }

    if (query.isActive()) {
        query.finish();
    }
    return scheduleType->typeID();
}

DScheduleType::Ptr ScheduleDataBase::getScheduleTypeByID(const QString &typeID, const int isDeleted)
{
    DScheduleType::Ptr type;
    QString strSql("SELECT                              \
                   st.typeID , st.typeName ,st.typeDisplayName ,        \
                   st.typePath ,st.typeColorID ,tc.ColorHex ,           \
                   tc.privilege as colorPri,st.description,             \
                   st.privilege as typePri,st.showState ,st.syncTag ,   \
                   st.dtCreate ,st.dtUpdate ,st.dtDelete ,st.isDeleted  \
               FROM                                                     \
                   scheduleType st                                      \
               inner join typeColor tc on                               \
                   st.typeColorID = tc.ColorID                          \
               WHERE                                                    \
                   st.typeID = ? AND st.isDeleted = ?");
    SqliteQuery query(m_database);
    if (query.prepare(strSql)) {
        query.addBindValue(typeID);
        query.addBindValue(isDeleted);
        if (query.exec() && query.next()) {
            type = DScheduleType::Ptr(new DScheduleType());
            type->setTypeID(typeID);
            type->setTypeName(query.value("typeName").toString());
            type->setDisplayName(query.value("typeDisplayName").toString());
            type->setTypePath(query.value("typePath").toString());
            DTypeColor color;
            color.setColorID(query.value("typeColorID").toString());
            color.setColorCode(query.value("ColorHex").toString());
            color.setPrivilege(static_cast<DTypeColor::Privilege>(query.value("colorPri").toInt()));
            type->setTypeColor(color);
            type->setDescription(query.value("description").toString());
            type->setPrivilege(static_cast<DScheduleType::Privilege>(query.value("typePri").toInt()));
            type->setSyncTag(query.value("syncTag").toInt());
            type->setDtCreate(dtFromString(query.value("dtCreate").toString()));
            type->setDtUpdate(dtFromString(query.value("dtUpdate").toString()));
            type->setDtDelete(dtFromString(query.value("dtDelete").toString()));
            type->setShowState(static_cast<DScheduleType::ShowState>(query.value("showState").toInt()));
            type->setDeleted(query.value("isDeleted").toInt());
            systemTypeTran(type);
        }
    } else {
        qCWarning(ServiceLogger) << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
    return type;
}

DScheduleType::List ScheduleDataBase::getScheduleTypeList(const int isDeleted)
{
    QString strSql("SELECT                              \
                   st.typeID , st.typeName ,st.typeDisplayName ,        \
                   st.typePath ,st.typeColorID ,tc.ColorHex ,           \
                   tc.privilege as colorPri,st.description,             \
                   st.privilege as typePri,st.showState ,st.syncTag ,   \
                   st.dtCreate ,st.dtUpdate ,st.dtDelete ,st.isDeleted  \
               FROM                                                     \
                   scheduleType st                                      \
               inner join typeColor tc on                               \
                   st.typeColorID = tc.ColorID                          \
               WHERE                                                    \
                   st.isDeleted = ?");
    DScheduleType::List typeList;
    SqliteQuery query(m_database);
    if (query.prepare(strSql)) {
        query.addBindValue(isDeleted);
        if (query.exec()) {
            while (query.next()) {
                DScheduleType::Ptr type = DScheduleType::Ptr(new DScheduleType());
                type->setTypeID(query.value("typeID").toString());
                type->setTypeName(query.value("typeName").toString());
                type->setDisplayName(query.value("typeDisplayName").toString());
                type->setTypePath(query.value("typePath").toString());
                DTypeColor color;
                color.setColorID(query.value("typeColorID").toString());
                color.setColorCode(query.value("ColorHex").toString());
                color.setPrivilege(static_cast<DTypeColor::Privilege>(query.value("colorPri").toInt()));
                type->setTypeColor(color);
                type->setDescription(query.value("description").toString());
                type->setPrivilege(static_cast<DScheduleType::Privilege>(query.value("typePri").toInt()));
                type->setSyncTag(query.value("syncTag").toInt());
                type->setDtCreate(dtFromString(query.value("dtCreate").toString()));
                type->setDtUpdate(dtFromString(query.value("dtUpdate").toString()));
                type->setDtDelete(dtFromString(query.value("dtDelete").toString()));
                type->setShowState(static_cast<DScheduleType::ShowState>(query.value("showState").toInt()));
                type->setDeleted(query.value("isDeleted").toInt());
                systemTypeTran(type);
                typeList.append(type);
            }
        } else {
            qCWarning(ServiceLogger) << "getScheduleTypeList error:" << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << "getScheduleTypeList error:" << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
    return typeList;
}

bool ScheduleDataBase::scheduleTypeByUsed(const QString &typeID, const int isDeleted)
{
    QString strSql("SELECT COUNT(scheduleTypeID) FROM schedules WHERE scheduleTypeID = ? AND isDeleted = ?;");
    SqliteQuery query(m_database);
    int typeCount = 0;
    if (query.prepare(strSql)) {
        query.addBindValue(typeID);
        query.addBindValue(isDeleted);

        if (query.exec() && query.next()) {
            typeCount = query.value(0).toInt();
        }
    } else {
        qCWarning(ServiceLogger) << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
    return typeCount;
}

bool ScheduleDataBase::deleteScheduleTypeByID(const QString &typeID, const int isDeleted)
{
    SqliteQuery query(m_database);
    QString strSql;
    if (isDeleted == 0) {
        //弱删除
        QDateTime &&dtCurrent = QDateTime::currentDateTime();
        strSql = QString("UPDATE scheduleType  SET  dtDelete='%1', isDeleted=1  WHERE typeID=?;")
                 .arg(dtToString(dtCurrent));
    } else {
        //真删除
        strSql = "DELETE FROM scheduleType WHERE typeID = ?";
    }
    bool res = false;
    if (query.prepare(strSql)) {
        query.addBindValue(typeID);
        res = query.exec();
        if (!res) {
            qCWarning(ServiceLogger) << "DELETE scheduleType error by typeID:" << typeID << " " << query.lastError();
        }
        if (query.isActive()) {
            query.finish();
        }
    } else {
        qCWarning(ServiceLogger) << "DELETE scheduleType error by typeID:" << typeID << " " << query.lastError();
    }

    return res;
}

bool ScheduleDataBase::restoreScheduleTypeByID(const QString &typeID)
{
    if (typeID.isEmpty()) {
        return false;
    }
    SqliteQuery query(m_database);
    if (!query.prepare(QStringLiteral(
            "UPDATE scheduleType SET dtDelete = NULL, isDeleted = 0 WHERE typeID = ?"))) {
        return false;
    }
    query.addBindValue(typeID);
    return query.exec() && query.numRowsAffected() == 1;
}

bool ScheduleDataBase::updateScheduleType(const DScheduleType::Ptr &scheduleType)
{
    bool res = false;
    SqliteQuery query(m_database);
    QString strSql("UPDATE scheduleType                     \
                       SET typeName=?, typeDisplayName=?, typePath=?, typeColorID=?, description=?,     \
                       privilege=?, showState=?, dtUpdate=?, dtDelete=?, isDeleted=?        \
                       WHERE typeID = ?");
    if (query.prepare(strSql)) {
        query.addBindValue(scheduleType->typeName());
        query.addBindValue(scheduleType->displayName());
        query.addBindValue(scheduleType->typePath());
        query.addBindValue(scheduleType->typeColor().colorID());
        query.addBindValue(scheduleType->description());
        query.addBindValue(int(scheduleType->privilege()));
        query.addBindValue(scheduleType->showState());
        query.addBindValue(dtConvert(scheduleType->dtUpdate()));
        query.addBindValue(dtConvert(scheduleType->dtDelete()));
        query.addBindValue(scheduleType->deleted());
        query.addBindValue(scheduleType->typeID());
        if (query.exec()) {
            res = true;
        } else {
            qCWarning(ServiceLogger) << "updateScheduleType error:" << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << "updateScheduleType error:" << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
    return res;
}

QString ScheduleDataBase::getFestivalTypeID()
{
    QString strSql("SELECT typeID FROM scheduleType WHERE  privilege = 0;");
    SqliteQuery query(m_database);
    QString typeID("");
    if (query.prepare(strSql)) {
        if (query.exec()) {
            if (query.next()) {
                typeID = query.value("typeID").toString();
            }
        } else {
            qCWarning(ServiceLogger) << "updateScheduleType error:" << query.lastError();
        }

    } else {
        qCWarning(ServiceLogger) << "updateScheduleType error:" << query.lastError();
    }
    if (query.isActive()) {
        query.finish();
    }

    return typeID;
}

QString ScheduleDataBase::getLocalTypeID()
{
    //默认日历按名字找：它的权限是 User，跟上面按 privilege = 0 找节假日的路子不同
    QString strSql("SELECT typeID FROM scheduleType WHERE typeName = 'Local' AND isDeleted = 0;");
    SqliteQuery query(m_database);
    QString typeID("");
    if (query.prepare(strSql)) {
        if (query.exec()) {
            if (query.next()) {
                typeID = query.value("typeID").toString();
            }
        } else {
            qCWarning(ServiceLogger) << "getLocalTypeID error:" << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << "getLocalTypeID error:" << query.lastError();
    }
    if (query.isActive()) {
        query.finish();
    }

    return typeID;
}

bool ScheduleDataBase::addTypeColor(const DTypeColor::Ptr &typeColor)
{
    if (typeColor.isNull())
        return false;
    return addTypeColor(*typeColor.data());
}

bool ScheduleDataBase::addTypeColor(DTypeColor &typeColor)
{
    QString strSql("INSERT INTO TypeColor                   \
                   (ColorID, ColorHex, privilege,dtCreate)           \
                   VALUES(:ColorID, :ColorHex, :privilege,:dtCreate)");

    //如果为空则创建颜色id
    if (typeColor.colorID().isEmpty()) {
        typeColor.setColorID(createUuid());
    }
    if (typeColor.dtCreate().isNull()) {
        typeColor.setDtCreate(QDateTime::currentDateTime());
    }
    SqliteQuery query(m_database);
    bool res = false;
    if (query.prepare(strSql)) {
        query.bindValue(":ColorID", typeColor.colorID());
        query.bindValue(":ColorHex", typeColor.colorCode());
        query.bindValue(":privilege", typeColor.privilege());
        query.bindValue(":dtCreate", dtToString(typeColor.dtCreate()));

        if (query.exec()) {
            res = true;
        } else {
            qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
    return res;
}

void ScheduleDataBase::deleteTypeColor(const QString &colorNo)
{
    QString strSql("DELETE FROM typeColor WHERE ColorID = ?;");
    SqliteQuery query(m_database);
    if (query.prepare(strSql)) {
        query.addBindValue(colorNo);
        if (!query.exec()) {
            qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
}

DTypeColor::List ScheduleDataBase::getSysColor()
{
    QString strSql("SELECT ColorID, ColorHex, privilege,dtCreate FROM typeColor WHERE  privilege =1;");
    SqliteQuery query(m_database);
    DTypeColor::List typeColorList;

    if (query.prepare(strSql) && query.exec()) {
        while (query.next()) {
            DTypeColor::Ptr color = DTypeColor::Ptr(new DTypeColor);
            color->setColorID(query.value("ColorID").toString());
            color->setColorCode(query.value("ColorHex").toString());
            color->setPrivilege(static_cast<DTypeColor::Privilege>(query.value("privilege").toInt()));
            color->setDtCreate(dtFromString(query.value("dtCreate").toString()));
            typeColorList.append(color);
        }
    } else {
        qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
    }
    if (query.isActive()) {
        query.finish();
    }
    return typeColorList;
}

bool ScheduleDataBase::upsertIcsSubscription(const IcsSubscription &sub)
{
    QString strSql("INSERT OR REPLACE INTO icsSubscription                              \
                   (typeID, url, refreshIntervalMin, lastSync, lastETag, dtCreate)      \
                   VALUES(?, ?, ?, ?, ?, ?)");
    //dtCreate为空时补上当前时间，与 addTypeColor 的行为保持一致
    QDateTime dtCreate = sub.dtCreate;
    if (dtCreate.isNull()) {
        dtCreate = QDateTime::currentDateTime();
    }
    SqliteQuery query(m_database);
    bool res = false;
    if (query.prepare(strSql)) {
        query.addBindValue(sub.typeID);
        query.addBindValue(sub.url);
        query.addBindValue(sub.refreshIntervalMin);
        query.addBindValue(dtToString(sub.lastSync));
        query.addBindValue(sub.lastETag);
        query.addBindValue(dtToString(dtCreate));
        if (query.exec()) {
            res = true;
        } else {
            qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
    return res;
}

ScheduleDataBase::IcsSubscription ScheduleDataBase::getIcsSubscription(const QString &typeID) const
{
    QString strSql("SELECT typeID, url, refreshIntervalMin, lastSync, lastETag, dtCreate \
                   FROM icsSubscription WHERE typeID = ?;");
    SqliteQuery query(m_database);
    IcsSubscription sub;
    if (query.prepare(strSql)) {
        query.addBindValue(typeID);
        if (query.exec()) {
            if (query.next()) {
                sub.typeID = query.value("typeID").toString();
                sub.url = query.value("url").toString();
                sub.refreshIntervalMin = query.value("refreshIntervalMin").toInt();
                sub.lastSync = dtFromString(query.value("lastSync").toString());
                sub.lastETag = query.value("lastETag").toString();
                sub.dtCreate = dtFromString(query.value("dtCreate").toString());
            }
        } else {
            qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
    return sub;
}

QVector<ScheduleDataBase::IcsSubscription> ScheduleDataBase::getIcsSubscriptionList() const
{
    QString strSql("SELECT typeID, url, refreshIntervalMin, lastSync, lastETag, dtCreate \
                   FROM icsSubscription;");
    SqliteQuery query(m_database);
    QVector<IcsSubscription> subList;
    if (query.prepare(strSql) && query.exec()) {
        while (query.next()) {
            IcsSubscription sub;
            sub.typeID = query.value("typeID").toString();
            sub.url = query.value("url").toString();
            sub.refreshIntervalMin = query.value("refreshIntervalMin").toInt();
            sub.lastSync = dtFromString(query.value("lastSync").toString());
            sub.lastETag = query.value("lastETag").toString();
            sub.dtCreate = dtFromString(query.value("dtCreate").toString());
            subList.append(sub);
        }
    } else {
        qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
    }
    if (query.isActive()) {
        query.finish();
    }
    return subList;
}

bool ScheduleDataBase::deleteIcsSubscription(const QString &typeID)
{
    QString strSql("DELETE FROM icsSubscription WHERE typeID = ?;");
    SqliteQuery query(m_database);
    bool res = false;
    if (query.prepare(strSql)) {
        query.addBindValue(typeID);
        if (query.exec()) {
            res = true;
        } else {
            qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
    return res;
}

void ScheduleDataBase::createDB()
{
    dbOpen();
    //这里用QFile来修改日历数据库文件的权限
    QFile file;
    file.setFileName(getDBPath());
    //如果不存在该文件则创建
    if (!file.exists() && m_database.open()) {
        m_database.close();
        qCDebug(ServiceLogger) << "Created new database file:" << getDBPath();
    }
    //将权限修改为600（对文件的所有者可以读写，其他用户不可读不可写）
    if (!file.setPermissions(QFile::WriteOwner | QFile::ReadOwner)) {
        qCWarning(ServiceLogger) << "Failed to set database file permissions:" << file.errorString();
    }

    if (m_database.open()) {
        SqliteQuery query(m_database);
        bool res = true;
        //日程表
        res = query.exec(sql_create_schedules);
        if (!res) {
            qCWarning(ServiceLogger) << "Failed to create schedules table:" << query.lastError().text();
        }

        //类型表
        res = query.exec(sql_create_scheduleType);
        if (!res) {
            qCWarning(ServiceLogger) << "Failed to create scheduleType table:" << query.lastError().text();
        }

        //颜色表
        res = query.exec(sql_create_typeColor);
        if (!res) {
            qCWarning(ServiceLogger) << "Failed to create typeColor table:" << query.lastError().text();
        }

        //ICS 订阅表
        res = query.exec(sql_create_icsSubscription);
        if (!res) {
            qCWarning(ServiceLogger) << "Failed to create icsSubscription table:" << query.lastError().text();
        }

        if (query.isActive()) {
            query.finish();
        }
    }
}

void ScheduleDataBase::initScheduleDB()
{
    //创建数据库时，需要初始化的日程数据
}

void ScheduleDataBase::initScheduleType()
{
    //创建数据库时，需要初始化的类型数据
    initSysType();
}

void ScheduleDataBase::initSysType()
{
    //本项目没有帐户体系，直接初始化本地日程类型数据
    QDateTime currentTime = QDateTime::currentDateTime();
    //添加节假日日程类型，不会展示在类型列表中
    DScheduleType::Ptr scheduleType(new DScheduleType);
    scheduleType->setTypeID(createUuid());
    scheduleType->setDtCreate(currentTime.addDays(-3));
    scheduleType->setPrivilege(DScheduleType::None);
    scheduleType->setTypeName("festival");
    scheduleType->setDisplayName("Holiday schedule type");
    scheduleType->setColorID(GFestivalColorID);
    scheduleType->setColorCode("#FF9436");
    scheduleType->setShowState(DScheduleType::Show);
    createScheduleType(scheduleType);

    //工作类型
    DScheduleType::Ptr workType(new DScheduleType);
    workType->setTypeID("107c369e-b13a-4d45-9ff3-de4eb3c0475b");
    workType->setDtCreate(currentTime.addDays(-2));
    workType->setPrivilege(DScheduleType::Read);
    workType->setTypeName("Work");
    workType->setDisplayName("Work");
    DTypeColor workColor;
    workColor.setColorID(GWorkColorID);
    workColor.setColorCode("#ff5e97");
    workColor.setPrivilege(DTypeColor::PriSystem);
    workType->setTypeColor(workColor);
    workType->setShowState(DScheduleType::Show);
    createScheduleType(workType);

    //生活
    DScheduleType::Ptr lifeType(new DScheduleType);
    lifeType->setTypeID("24cf3ae3-541d-487f-83df-f068416b56b6");
    lifeType->setDtCreate(currentTime.addDays(-1));
    lifeType->setPrivilege(DScheduleType::Read);
    lifeType->setTypeName("Life");
    lifeType->setDisplayName("Life");
    DTypeColor lifeColor;
    lifeColor.setColorID(GLifeColorID);
    lifeColor.setColorCode("#5d51ff");
    lifeColor.setPrivilege(DTypeColor::PriSystem);
    lifeType->setTypeColor(lifeColor);
    lifeType->setShowState(DScheduleType::Show);
    createScheduleType(lifeType);

    //其他
    DScheduleType::Ptr otherType(new DScheduleType);
    otherType->setTypeID("403bf009-2005-4679-9c76-e73d9f83a8b4");
    otherType->setDtCreate(currentTime);
    otherType->setPrivilege(DScheduleType::Read);
    otherType->setTypeName("Other");
    otherType->setDisplayName("Other");
    DTypeColor otherColor;
    otherColor.setColorID(GOtherColorID);
    otherColor.setColorCode("#5bdd80");
    otherColor.setPrivilege(DTypeColor::PriSystem);
    otherType->setTypeColor(otherColor);
    otherType->setShowState(DScheduleType::Show);
    createScheduleType(otherType);

    //本地日历。参考实现里新建日程要选一个帐户，类型是帐户下面的东西；
    //本项目没有帐户体系，用户需要有一个开箱即用、可读写的默认日历，
    //所以除了上面三个系统类型，再建一个归属于用户自己的。
    //ID 取的是参考实现本地帐户的固定 ID，从 dde-calendar 迁过来的数据能对上。
    DScheduleType::Ptr localType(new DScheduleType);
    localType->setTypeID("7e5c1a3b-9d47-4f2e-8b61-0a3c5e7f9d21");
    localType->setDtCreate(currentTime);
    localType->setPrivilege(DScheduleType::User);
    localType->setTypeName("Local");
    localType->setDisplayName("Local");
    DTypeColor localColor;
    localColor.setColorID("5bf13e88-e99f-4975-80a8-149fe0a315e3");
    localColor.setColorCode("#4293ff");
    localColor.setPrivilege(DTypeColor::PriSystem);
    localType->setTypeColor(localColor);
    localType->setShowState(DScheduleType::Show);
    createScheduleType(localType);
}

void ScheduleDataBase::systemTypeTran(const DScheduleType::Ptr &type)
{
    //如果类型为默认类型（参考实现还要求本地帐户，本项目没有帐户体系）
    if (type->privilege() == DScheduleType::Privilege::Read) {
        if (type->typeName() == "Work") {
            type->setDisplayName(tr("Work"));
        } else if (type->typeName() == "Life") {
            type->setDisplayName(tr("Life"));
        } else if (type->typeName() == "Other") {
            type->setDisplayName(tr("Other"));
        }
    } else if (type->typeName() == "Local") {
        //本项目自建的默认日历，权限是 User（可读写），不适用上面那条判断，
        //但显示名同样要走翻译。
        type->setDisplayName(tr("Local Calendar"));
    }
}

void ScheduleDataBase::initTypeColor()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    int index = -10;

    QMap<QString, QString>::const_iterator iter = GTypeColor.constBegin();
    for (; iter != GTypeColor.constEnd(); ++iter) {
        DTypeColor::Ptr typeColor(new DTypeColor);
        typeColor->setDtCreate(currentTime.addDays(++index));
        typeColor->setPrivilege(DTypeColor::PriSystem);
        typeColor->setColorCode(iter.value());
        typeColor->setColorID(iter.key());
        addTypeColor(typeColor);
    }
}
