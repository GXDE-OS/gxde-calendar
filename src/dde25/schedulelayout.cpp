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
 * 移植自 dde-calendar（见头文件说明）。
 */

#include "schedulelayout.h"

#include <QTime>

#include <algorithm>

namespace DDE25 {

namespace {

// 日程块相对格子顶部往下让多少，给日期数字腾地方（参考实现 schedule_Item_Y）
const int kScheduleItemY = 31;
// 月视图块右侧留白（对应 computePos 里的 -11）
const int kMonthBlockGap = 11;

} // namespace

int monthRowsPerCell(int sceneHeight, int itemHeight)
{
    if (itemHeight <= 0) {
        return 0;
    }
    // 参考实现：m_cNum = ((高度/6) + 0.5 - 31) / (行高 + 1)
    const double cellHeight = sceneHeight / 6.0;
    const int rows = static_cast<int>((cellHeight + 0.5 - kScheduleItemY) / (itemHeight + 1));
    return rows > 0 ? rows : 0;
}

QVector<MonthBlock> layoutMonthBlocks(const QMap<QDate, DSchedule::List> &data,
                                      const QDate &beginDate,
                                      int sceneWidth, int sceneHeight, int itemHeight)
{
    QVector<MonthBlock> result;

    const int maxNum = monthRowsPerCell(sceneHeight, itemHeight);
    if (maxNum < 1 || sceneWidth <= 0 || sceneHeight <= 0) {
        return result;
    }

    const double cellWidth = sceneWidth / 7.0;
    const double cellHeight = sceneHeight / 6.0;
    const int rows = static_cast<int>(sceneHeight / cellHeight);

    for (int week = 0; week < rows; ++week) {
        const QDate weekBegin = beginDate.addDays(week * 7);
        const QDate weekEnd = weekBegin.addDays(6);

        // ---- 收集这一周内要画的日程，跨周的裁到本周 ----
        QVector<DSchedule::Ptr> schedules;
        for (int i = 0; i <= weekBegin.daysTo(weekEnd); ++i) {
            const DSchedule::List &dayList = data.value(weekBegin.addDays(i));
            for (const DSchedule::Ptr &info : dayList) {
                if (info.isNull()) {
                    continue;
                }
                // 同一条日程（重复日程按天展开后仍指向同一个对象）在一周里只画一次
                bool have = false;
                for (const DSchedule::Ptr &p : schedules) {
                    if (p == info) {
                        have = true;
                        break;
                    }
                }
                if (!have) {
                    schedules.append(info);
                }
            }
        }

        struct RangeInfo {
            QDate bdate;
            QDate edate;
            bool state = false;   // true 表示「还有 N 项」
            int num = 0;
            DSchedule::Ptr tData;
        };

        QVector<RangeInfo> rangeList;
        QVector<int> columnCount;
        columnCount.fill(0, 7);

        for (const DSchedule::Ptr &s : schedules) {
            QDate tBegin = s->dtStart().date();
            QDate tEnd = s->dtEnd().date();
            if (tEnd < weekBegin || tBegin > weekEnd) {
                continue;
            }
            if (tBegin < weekBegin) {
                tBegin = weekBegin;
            }
            if (tEnd > weekEnd) {
                tEnd = weekEnd;
            }

            RangeInfo info;
            info.bdate = tBegin;
            info.edate = tEnd;
            info.tData = s;
            info.state = false;
            rangeList.append(info);

            // 这条日程占用了哪几列，列计数用来判断「要不要收成 N more」
            const int pos = static_cast<int>(weekBegin.daysTo(tBegin));
            const int span = static_cast<int>(tBegin.daysTo(tEnd));
            for (int j = pos; j < pos + span + 1; ++j) {
                if (j >= 0 && j < columnCount.size()) {
                    ++columnCount[j];
                }
            }
        }

        // 排序：先按起始日期，同日按跨度长的在前
        std::sort(rangeList.begin(), rangeList.end(), [](const RangeInfo &a, const RangeInfo &b) {
            if (a.bdate == b.bdate) {
                const qint64 la = a.bdate.daysTo(a.edate);
                const qint64 lb = b.bdate.daysTo(b.edate);
                if (la == lb) {
                    return a.tData < b.tData;
                }
                return la > lb;
            }
            return a.bdate < b.bdate;
        });

        // ---- 首次适应行打包（参考实现 sortAndFilter）----
        QVector<QVector<RangeInfo>> packedRows;
        QVector<QVector<bool>> fill;
        {
            QVector<bool> rowTemplate;
            rowTemplate.fill(false, 7);
            fill.fill(rowTemplate, maxNum);
        }

        for (const RangeInfo &cur : rangeList) {
            int postion = static_cast<int>(weekBegin.daysTo(cur.bdate));
            const int end = static_cast<int>(weekBegin.daysTo(cur.edate));
            int row = 0;
            int pos = postion;
            int count = 0;
            int scheduleRow = row;

            for (; postion < end + 1; ++postion) {
                if (row == maxNum) {
                    row = 0;
                    pos = postion;
                }
                while (row < maxNum) {
                    if (packedRows.size() < (row + 1)) {
                        packedRows.append(QVector<RangeInfo>());
                    }
                    if (!fill[row][postion]) {
                        // 这一列本来就放不下，且已经到了最后一行：收成「还有 N 项」
                        if (columnCount[postion] > maxNum && row >= maxNum - 1) {
                            fill[row][postion] = true;
                            if (pos != postion) {
                                RangeInfo shown;
                                shown.bdate = weekBegin.addDays(pos);
                                shown.edate = weekBegin.addDays(postion - 1);
                                shown.state = false;
                                shown.tData = cur.tData;
                                packedRows[row].append(shown);
                            }
                            RangeInfo more;
                            more.bdate = weekBegin.addDays(postion);
                            more.edate = more.bdate;
                            more.num = columnCount[postion] - maxNum + 1;
                            more.state = true;
                            packedRows[row].append(more);
                            pos = postion;
                            // for 循环末尾会 ++，这里先 -- 保证下一轮还停在这一列
                            --postion;
                            row = 0;
                            count = 0;
                        } else {
                            fill[row][postion] = true;
                            ++count;
                            scheduleRow = row;
                        }
                        break;
                    } else {
                        if (count > 0 && pos != postion) {
                            RangeInfo shown;
                            shown.bdate = weekBegin.addDays(pos);
                            shown.edate = weekBegin.addDays(postion - 1);
                            shown.state = false;
                            shown.tData = cur.tData;
                            packedRows[scheduleRow].append(shown);
                        }
                        ++row;
                    }
                }
            }
            if (pos < 7 && count > 0) {
                RangeInfo shown;
                shown.bdate = weekBegin.addDays(pos);
                shown.edate = weekBegin.addDays(postion - 1);
                shown.state = false;
                shown.tData = cur.tData;
                if (scheduleRow < packedRows.size()) {
                    packedRows[scheduleRow].append(shown);
                }
            }
        }

        // ---- 行内块换算成绝对矩形 ----
        for (int r = 0; r < packedRows.size(); ++r) {
            const int cNum = r + 1;
            for (const RangeInfo &info : packedRows.at(r)) {
                const int bcol = static_cast<int>(weekBegin.daysTo(info.bdate)) % 7;
                const int ecol = static_cast<int>(weekBegin.daysTo(info.edate)) % 7;

                MonthBlock block;
                const int w = static_cast<int>((ecol - bcol + 1) * cellWidth - kMonthBlockGap);
                const int h = itemHeight + 2;
                const int x = static_cast<int>(bcol * cellWidth + 5);
                const int y = static_cast<int>(cellHeight * week + kScheduleItemY + (cNum - 1) * h);
                block.rect = QRect(x, y, w, h);
                block.isMore = info.state;
                block.moreCount = info.num;
                block.schedule = info.tData;
                // 跨天块取被裁到的那一段的起始日；键盘导航和右键新建都要用它定位
                block.date = info.bdate;
                result.append(block);
            }
        }
    }

    return result;
}

QVector<ScheduleCluster> classifyOverlaps(const DSchedule::List &daySchedules, int minSecs)
{
    QVector<ScheduleCluster> info;

    DSchedule::List list;
    for (const DSchedule::Ptr &s : daySchedules) {
        if (!s.isNull()) {
            list.append(s);
        }
    }
    if (list.isEmpty()) {
        return info;
    }

    // 参考实现这里用 std::sort 的默认比较，也就是按 QSharedPointer 的地址排——
    // 顺序跟日程本身的先后无关，每次重启程序都可能换个样。这里改成按开始时间排，
    // 让并排块的左右顺序稳定且符合直觉（早的靠左）。
    std::sort(list.begin(), list.end(), [](const DSchedule::Ptr &a, const DSchedule::Ptr &b) {
        if (a->dtStart() == b->dtStart()) {
            return a->dtEnd() < b->dtEnd();
        }
        return a->dtStart() < b->dtStart();
    });

    for (const DSchedule::Ptr &s : list) {
        QDateTime endTime = s->dtEnd();
        const QDateTime begTime = s->dtStart();

        // 当天的短日程会被拉长到最短时长，否则画出来就是一条线看不见
        if (minSecs > 0 && begTime.date().daysTo(endTime.date()) == 0
            && begTime.time().secsTo(endTime.time()) < minSecs) {
            endTime = begTime.addSecs(minSecs);
        }
        // 结束时间是 00:00:00 说明它是「到当天零点为止」，减一秒归到前一天
        if (endTime.time().hour() == 0 && endTime.time().second() == 0) {
            endTime = endTime.addSecs(-1);
        }

        // 找出所有跟它时间上相交的组（可能不止一个，跨过去就把它们并起来）
        QVector<int> containIndex;
        for (int i = 0; i < info.size(); ++i) {
            const bool startInside = s->dtStart() >= info.at(i).beginDate
                && s->dtStart() <= info.at(i).endDate;
            const bool endInside = endTime >= info.at(i).beginDate
                && endTime <= info.at(i).endDate;
            if (startInside || endInside) {
                containIndex.append(i);
            }
        }

        if (containIndex.isEmpty()) {
            ScheduleCluster firstschedule;
            firstschedule.beginDate = s->dtStart();
            firstschedule.endDate = endTime;
            firstschedule.members.append(s);
            info.append(firstschedule);
            continue;
        }

        // 并进第一个相交的组，再把后面相交的组整个搬进来
        ScheduleCluster &scheduleInfo = info[containIndex.first()];
        for (int i = 1; i < containIndex.size(); ++i) {
            const int index = containIndex.at(i);
            if (info.at(index).beginDate < scheduleInfo.beginDate) {
                scheduleInfo.beginDate = info.at(index).beginDate;
            }
            if (info.at(index).endDate > scheduleInfo.endDate) {
                scheduleInfo.endDate = info.at(index).endDate;
            }
            scheduleInfo.members.append(info.at(index).members);
        }
        // 倒着删，下标才不会因为前一次删除而错位
        for (int i = containIndex.size() - 1; i > 0; --i) {
            info.removeAt(containIndex.at(i));
        }

        // 并入之后组的范围变大了，可能又跟别的组相交，这里再收一次尾
        if (endTime > scheduleInfo.endDate) {
            scheduleInfo.endDate = endTime;
        }
        if (s->dtStart() < scheduleInfo.beginDate) {
            scheduleInfo.beginDate = s->dtStart();
        }
        scheduleInfo.members.append(s);
    }

    // 合并过程只保证「组之间不再相交」，组内顺序要在这里重新捋一遍
    for (ScheduleCluster &cluster : info) {
        std::sort(cluster.members.begin(), cluster.members.end(),
                  [](const DSchedule::Ptr &a, const DSchedule::Ptr &b) {
                      if (a->dtStart() == b->dtStart()) {
                          return a->dtEnd() < b->dtEnd();
                      }
                      return a->dtStart() < b->dtStart();
                  });
    }

    return info;
}

QVector<DSchedule::List> packAllDayRows(const DSchedule::List &allDaySchedules,
                                        const QDate &beginDate, const QDate &endDate)
{
    QVector<DSchedule::List> rows;

    struct Item {
        QDate bdate;
        QDate edate;
        DSchedule::Ptr data;
    };
    QVector<Item> items;

    for (const DSchedule::Ptr &s : allDaySchedules) {
        if (s.isNull()) {
            continue;
        }
        QDate b = s->dtStart().date();
        QDate e = s->dtEnd().date();
        if (e < beginDate || b > endDate) {
            continue;
        }
        if (b < beginDate) {
            b = beginDate;
        }
        if (e > endDate) {
            e = endDate;
        }
        items.append({b, e, s});
    }

    std::sort(items.begin(), items.end(), [](const Item &a, const Item &b) {
        if (a.bdate == b.bdate) {
            return a.bdate.daysTo(a.edate) > b.bdate.daysTo(b.edate);
        }
        return a.bdate < b.bdate;
    });

    // 逐行试放：这一行已有的日程都跟它不重叠才放得下
    for (const Item &item : items) {
        bool placed = false;
        for (DSchedule::List &row : rows) {
            bool clash = false;
            for (const DSchedule::Ptr &other : row) {
                QDate ob = other->dtStart().date();
                QDate oe = other->dtEnd().date();
                if (ob < beginDate) {
                    ob = beginDate;
                }
                if (oe > endDate) {
                    oe = endDate;
                }
                if (!(item.edate < ob || item.bdate > oe)) {
                    clash = true;
                    break;
                }
            }
            if (!clash) {
                row.append(item.data);
                placed = true;
                break;
            }
        }
        if (!placed) {
            DSchedule::List row;
            row.append(item.data);
            rows.append(row);
        }
    }

    return rows;
}

} // namespace DDE25
