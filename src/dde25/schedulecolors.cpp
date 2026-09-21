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
 * 改自 dde-calendar（src/calendar-client/src/dataManage/scheduledatamanage.cpp）。
 */

#include "schedulecolors.h"

#include <QApplication>
#include <QPalette>

#include "schedule/calendarservice.h"
#include "schedule/dscheduletype.h"

namespace DDE25 {

CSchedulesColor scheduleColorByType(const QString &typeID)
{
    CSchedulesColor color;

    QColor typeColor;
    const DScheduleType::Ptr type = CalendarService::instance()->getScheduleTypeByID(typeID);
    if (!type.isNull()) {
        typeColor = QColor(type->getColorCode());
    }

    // 类型查不到（或颜色串坏了）时给中性灰，否则 alpha 0.2 的黑色等于什么都看不见
    if (!typeColor.isValid()) {
        typeColor = QColor("#717171");
    }

    // 四态透明度与参考实现一致
    color.orginalColor = typeColor;
    color.normalColor = typeColor;
    color.normalColor.setAlphaF(0.2);
    color.pressColor = typeColor;
    color.pressColor.setAlphaF(0.35);
    color.hoverColor = typeColor;
    color.hoverColor.setAlphaF(0.3);
    color.hightColor = typeColor;
    color.hightColor.setAlphaF(0.35);
    return color;
}

QColor scheduleTextColor()
{
    // 参考实现取的是 DGuiApplicationHelper 的 applicationPalette()。本项目
    // dde25common.cpp 里有意识地不走 DGuiApplicationHelper（见 getSystemActiveColor），
    // 这里跟它保持一致，直接用 qApp 的调色板。
    return qApp->palette().color(QPalette::Text);
}

} // namespace DDE25
