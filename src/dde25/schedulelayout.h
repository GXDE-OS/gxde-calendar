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
 * This file is ported from DDE calendar tag 6.6.0
 * Minimal modification is applied to make the class build against
 * DTK2Widget-Qt6.
 * ----------------------------------------------------------------------------
 * 移植自 dde-calendar：
 *   月视图排布   src/calendar-client/src/widget/monthWidget/monthscheduleview.cpp
 *                （CWeekScheduleView::updateSchedule / sortAndFilter / addShowSchedule，
 *                  以及 CMonthScheduleView::computePos）
 *   周/日视图排布 src/calendar-client/src/view/graphicsview.cpp
 *                （CScheduleClassificationType 重叠分簇 + addScheduleItem）
 *
 * 参考实现把这些算法散在几个 QWidget 子类里，跟具体控件绑得太死没法单测。
 * 这里抽成不依赖任何控件、只吃数据吐矩形的自由函数，视图层拿结果直接 addItem。
 *
 * 周/日视图的「并排矩形」不在这里算——那一套（最小列宽 27px、最小块高 20px、
 * 跨天裁剪、坐标反查时间）已经在 schedulecoormanage.* 里整棵移植好了，
 * 这里只负责把重叠的日程分好组，几何交给它，免得两份实现走偏。
 */

#ifndef SCHEDULELAYOUT_H
#define SCHEDULELAYOUT_H

#include "schedule/dschedule.h"

#include <QDate>
#include <QDateTime>
#include <QMap>
#include <QRect>
#include <QVector>

namespace DDE25 {

// 月视图里一块要画的东西：一条日程，或一个「还有 N 项」
struct MonthBlock {
    QRect rect;
    DSchedule::Ptr schedule;   // isMore 为 false 时有效
    bool isMore = false;
    int moreCount = 0;
    // 这一块的起始日期（跨天块取被裁到的那一天）
    QDate date;
};

// 时间上互相重叠的一组日程，组内要在一列里并排画
struct ScheduleCluster {
    DSchedule::List members;
    QDateTime beginDate;   // 这一组覆盖到的最早开始时间
    QDateTime endDate;     // 这一组覆盖到的最晚结束时间
};

/**
 * @brief layoutMonthBlocks   排布月视图 42 格里的日程块
 *
 * @param data          按天分组的日程（querySchedulesWithParameter 的结果）
 * @param beginDate     42 格中第一格的日期
 * @param sceneWidth    42 格绘图区的宽
 * @param sceneHeight   42 格绘图区的高
 * @param itemHeight    一行日程块的高度
 *
 * 每格能放几行由格子高度决定；放不下的列会在最后一行收成「还有 N 项」。
 */
QVector<MonthBlock> layoutMonthBlocks(const QMap<QDate, DSchedule::List> &data,
                                      const QDate &beginDate,
                                      int sceneWidth, int sceneHeight, int itemHeight);

// 一格高度里能排下几行日程
int monthRowsPerCell(int sceneHeight, int itemHeight);

/**
 * @brief classifyOverlaps   把一天里的日程按时间重叠关系分组
 *
 * 返回的每组内部按开始时间排序，调用方拿组内下标去 CScheduleCoorManage
 * 的 getDrawRegion(date, start, end, index, count, maxNum, viewType) 算并排矩形。
 *
 * @param daySchedules  这一天要画的日程（已按调用方过滤、已排序）
 * @param minSecs       最小高度换算出来的最短时长；同一天内短于它的日程会被拉长，
 *                      免得画出来是一条看不见的线。<=0 表示不做这个拉伸。
 */
QVector<ScheduleCluster> classifyOverlaps(const DSchedule::List &daySchedules, int minSecs);

/**
 * @brief packAllDayRows   把全天日程按「能不重叠就不重叠」打包成若干行
 *
 * 返回每一行放哪些日程，行号即第几行（0 起）。
 */
QVector<DSchedule::List> packAllDayRows(const DSchedule::List &allDaySchedules,
                                        const QDate &beginDate, const QDate &endDate);

} // namespace DDE25

#endif // SCHEDULELAYOUT_H
