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
 * Verbatim apart from includes.
 * ----------------------------------------------------------------------------
 * 移植自 dde-calendar（src/calendar-common/src/dschedulequerypar.h）。
 */
#ifndef DSCHEDULEQUERYPAR_H
#define DSCHEDULEQUERYPAR_H

#include <QDateTime>
#include <QString>
#include <QSharedPointer>

//查询日程相关参数
class DScheduleQueryPar
{
public:
    typedef QSharedPointer<DScheduleQueryPar> Ptr;

    enum QueryType {
        Query_None,
        Query_RRule, //查询重复规则
        Query_Top, //查询前多少个
        Query_ScheduleID //查询日程ID
    };

    enum RRuleType {
        RRule_None, //从不
        RRule_Day, //每天
        RRule_Work, //每工作日
        RRule_Week, //每周
        RRule_Month, //每月
        RRule_Year, //每年
    };

    DScheduleQueryPar();

    QDateTime dtStart() const;
    void setDtStart(const QDateTime &dtStart);

    QDateTime dtEnd() const;
    void setDtEnd(const QDateTime &dtEnd);

    QString key() const;
    void setKey(const QString &key);

    static DScheduleQueryPar::Ptr fromJsonString(const QString &queryStr);
    static QString toJsonString(const DScheduleQueryPar::Ptr &queryPar);

    QueryType queryType() const;
    void setQueryType(const QueryType &queryType);

    int queryTop() const;
    void setQueryTop(int queryTop);

    RRuleType rruleType() const;
    void setRruleType(const RRuleType &rruleType);

private:
    QString m_key; //查询关键字，如果查询类型为日程ID，则表示日程ID
    int m_queryTop; //查询范围内前多少个日程
    RRuleType m_rruleType; //查询对应重复规则的日程
    QueryType m_queryType; //查询的类型
    QDateTime m_dtStart; //查询的起始时间
    QDateTime m_dtEnd; //查询的截止时间
};

Q_DECLARE_METATYPE(DScheduleQueryPar)
Q_DECLARE_METATYPE(DScheduleQueryPar::Ptr)

#endif // DSCHEDULEQUERYPAR_H
