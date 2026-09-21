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
 * 移植/改编自 dde-calendar（src/calendar-client），供 GXDE Calendar 的 DDE 25 视图使用。
 */

#ifndef DDE25COMMON_H
#define DDE25COMMON_H

#include <QColor>
#include <QDate>
#include <QHash>
#include <QIcon>
#include <QSet>
#include <QString>
#include <QVector>

#include "calendardbus.h"

namespace DDE25 {

// 主题类型：0/1 浅色，2 深色。与 dde-calendar 里 setTheMe(int type) 的约定保持一致。
int themeType();

// 系统高亮色。dde-calendar 取的是 DGuiApplicationHelper 的 highlight() 颜色，
// 这里用 Qt 自身的 highlight 色，避免额外依赖。
QColor systemActiveColor();

// 翻页箭头（日视图迷你月历、周视图周数条上的 < >）。
// dde-calendar 用 DIconButton(DStyle::SP_ArrowLeft)，DTK 画出来是一根 3px 粗的实心箭头，
// 移植到 Qt6 后换成从 dde-calendar 那套 previous_/next_ 箭头抠出来的细描边版本。
// type 为 setTheMe 的主题类型：2 用白色描边，其余用深灰。
QIcon navArrowIcon(bool next, int type);

// gxde-calendar 的 Week 枚举（Monday = 6 … Sunday = 0）转成 Qt::DayOfWeek。
Qt::DayOfWeek fromGxdeWeekday(int weekday);

// 某天所在周的周一（按 firstDay 指定的一周起始日推算）。
QDate firstDayOfWeek(const QDate &date, Qt::DayOfWeek firstDay);

// 一年中的第几周。算法与 dde-calendar 的 CalendarManager::getWeekNumOfYear 一致：
// 以「该周最后一天所在年」为准，第 1 周从该年第一个显示周的第一天算起。
int weekNumOfYear(const QDate &date, Qt::DayOfWeek firstDay);

// 一周的 7 天（从 firstDay 起算）。
QVector<QDate> weekDates(const QDate &date, Qt::DayOfWeek firstDay);

// 农历数据：按整月一次性拉取并缓存，绘制时只读缓存，避免在 paint 里发 DBus 调用。
class LunarCache
{
public:
    static LunarCache *instance();

    // 确保 year/month 的农历数据已就绪，必要时发一次 DBus 调用。
    // 结果会累加进缓存（月视图的 42 格可能跨 3 个月）。
    // 返回 true 表示缓存内容发生了变化（调用方据此决定是否重绘）。
    bool ensureMonth(int year, int month);

    // 取某天的农历信息，未缓存时返回默认构造的空值。
    CaLunarDayInfo info(const QDate &date) const;

    // 单元格上显示的农历文本：初一显示月名，有节气则优先显示节气。
    QString lunarText(const QDate &date) const;

    // 清空缓存（黄历服务数据刷新后调用）。
    void clear();

private:
    LunarCache() = default;

    // key 为 yyyyMM
    bool isMonthLoaded(int year, int month) const;

    QHash<QDate, CaLunarDayInfo> m_cache;
    QSet<int> m_loadedMonths;
};

} // namespace DDE25

#endif // DDE25COMMON_H
