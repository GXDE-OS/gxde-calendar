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
#include <QTimer>
#include <QVector>

#include <functional>

#include "calendardbus.h"

namespace DDE25 {

// 主题类型：0/1 浅色，2 深色。与 dde-calendar 里 setTheMe(int type) 的约定保持一致。
int themeType();

// 系统高亮色。dde-calendar 取的是 DGuiApplicationHelper 的 highlight() 颜色，
// 这里用 Qt 自身的 highlight 色，避免额外依赖。
QColor systemActiveColor();

// 翻页箭头（年视图工具栏、日视图迷你月历、周视图周数条上的 < >）。
// dde-calendar 用 DIconButton(DStyle::SP_ArrowLeft)，DTK 画出来是一根 3px 粗的实心箭头，
// 移植到 Qt6 后改用细描边折线：45 度、两端各 6px、描边 1（渲染出来是 1px 细边），
// 尺寸照 dde-calendar 侧栏那个箭头量的（见 resources/icon/{previous,next}_nav*.svg）。
// type 为 setTheMe 的主题类型：2 用 DTK2 深色主题的 #DDDDDD，其余用 #303030。
QIcon navArrowIcon(bool next, int type);

// 分栏之间 1px 分隔线的样式，取自 gxde-file-manager 主题里的 QSplitter::handle
// （gxde-file-manager-lib/themes/{light,dark}/DFileManagerWindow.theme：
//  background-color: rgba(0, 0, 0, 0.1); width/height: 1px; 深浅主题同值）。
// 侧栏与 M/W/D 之间、日视图左右分栏之间都用它，配合 setFixedWidth/Height(1)。
QString separatorStyleSheet();

// gxde-calendar 的 Week 枚举（Monday = 6 … Sunday = 0）转成 Qt::DayOfWeek。
Qt::DayOfWeek fromGxdeWeekday(int weekday);

// 某天所在周的周一（按 firstDay 指定的一周起始日推算）。
QDate firstDayOfWeek(const QDate &date, Qt::DayOfWeek firstDay);

// 一年中的第几周。算法与 dde-calendar 的 CalendarManager::getWeekNumOfYear 一致：
// 以「该周最后一天所在年」为准，第 1 周从该年第一个显示周的第一天算起。
int weekNumOfYear(const QDate &date, Qt::DayOfWeek firstDay);

// 一周的 7 天（从 firstDay 起算）。
QVector<QDate> weekDates(const QDate &date, Qt::DayOfWeek firstDay);

// M/W/D 三个视图的滚轮节流间隔（毫秒）：一次翻月/换天要重建整屏，
// 间隔比重建耗时略大即可——太短会堆积，太长会觉得滚动没跟上手。
constexpr int kWheelCooldownMs = 160;

// 滚轮一档的角度量：QWheelEvent::angleDelta() 以八分之一度为单位，120 就是滚轮的一格
constexpr int kWheelUnit = 120;

// 滚轮节流器：把一串滚轮事件按「格」累计，再合并成一次跳转。
// 翻月/换天要重建整屏（Debug 构建下一次约 80~100ms），一档一次重建的话，连续滚动就会
// 堆积成几百毫秒的卡顿（档数 × 重建耗时）。这里第一下立刻响应，之后落在冷却期内的档位
// 只累计，冷却结束时合并跳一次——既不丢滚动量，也不会堆积。
class WheelStepper
{
public:
    // handler 收到的是累计档数：滚轮向上为 +，向下为 -（正负同传入 delta 的符号）
    // cooldownMs 是两次跳转之间的最小间隔，翻页动画比它短为宜，免得动画被中途打断
    explicit WheelStepper(std::function<void(int)> handler, int cooldownMs);

    // 滚轮入口：delta 为 QWheelEvent::angleDelta() 的分量，按角度凑够一格才算一档
    void step(int delta);

    // 离散入口（键盘连发等）：一次调用就是一档，不参与角度累计
    void nudge(int steps);

private:
    // 记档：冷却期内只累计，否则立刻跳并开始冷却
    void push(int steps);
    void flush();

    std::function<void(int)> m_handler;
    QTimer m_cooldown;
    int m_pending = 0;    // 冷却期内攒下的档数
    int m_remainder = 0;  // 不足一档的角度余量
};

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
