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
 * Adapted from the DDE calendar schedule colour helper, modified to adapt the
 * GXDE calendar.
 * ----------------------------------------------------------------------------
 * 改自 dde-calendar 的 CScheduleDataManage::getScheduleColorByType /
 * getTextColor（src/calendar-client/src/dataManage/scheduledatamanage.cpp）。
 *
 * 参考实现把那套配色挂在 CScheduleDataManage 单例上，而那个单例同时还管着
 * 主题、选中态、拖拽态等一堆状态。本项目只需要「按日程类型取一组颜色」这一件事，
 * 因此抽成自由函数，不再引入单例。
 */

#ifndef SCHEDULECOLORS_H
#define SCHEDULECOLORS_H

#include <QColor>
#include <QString>

namespace DDE25 {

// 一个日程块在四种状态下的颜色。原色即类型颜色本身。
struct CSchedulesColor {
    QColor normalColor;
    QColor hoverColor;
    QColor pressColor;
    QColor hightColor;
    QColor orginalColor;
};

/**
 * @brief scheduleColorByType   按日程类型 ID 取配色
 *
 * 颜色取自类型挂着的调色板（ScheduleDataBase 里 join 出来的 ColorHex）。
 * 类型查不到时给一个中性灰，避免整块画成透明看不见。
 */
CSchedulesColor scheduleColorByType(const QString &typeID);

// 日程块上的文字色，跟随应用调色板
QColor scheduleTextColor();

} // namespace DDE25

#endif // SCHEDULECOLORS_H
