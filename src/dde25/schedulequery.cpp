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
 * 本文件为 GXDE 日历原创。
 */

#include "schedulequery.h"

#include "schedule/calendarservice.h"
#include "schedule/dschedulequerypar.h"

#include <QTime>

namespace DDE25 {

QMap<QDate, DSchedule::List> querySchedules(const QDate &begin, const QDate &end)
{
    if (!begin.isValid() || !end.isValid() || begin > end) {
        return {};
    }

    DScheduleQueryPar::Ptr par(new DScheduleQueryPar);
    // Query_None 会把跨天的日程在区间内每一天都展开一份，正是界面要的效果
    par->setQueryType(DScheduleQueryPar::Query_None);
    par->setDtStart(QDateTime(begin, QTime(0, 0, 0)));
    par->setDtEnd(QDateTime(end, QTime(23, 59, 59)));
    return CalendarService::instance()->querySchedulesWithParameter(par);
}

} // namespace DDE25
