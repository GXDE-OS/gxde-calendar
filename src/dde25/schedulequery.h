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
 * 本文件为 GXDE 日历原创：把「按日期区间取日程」这一件事收在一处，
 * 免得月/周/日三个视图各自拼查询参数拼出不一致的区间。
 */

#ifndef SCHEDULEQUERY_H
#define SCHEDULEQUERY_H

#include "schedule/dschedule.h"

#include <QDate>
#include <QMap>

namespace DDE25 {

/**
 * @brief querySchedules   取 [begin, end] 闭区间内的日程，按天分组
 *
 * 区间两端都含当天（结束日取到 23:59:59）。重复日程已经按天展开，
 * 隐藏的日历也已经在数据层过滤掉了，界面直接按天取用即可。
 */
QMap<QDate, DSchedule::List> querySchedules(const QDate &begin, const QDate &end);

} // namespace DDE25

#endif // SCHEDULEQUERY_H
