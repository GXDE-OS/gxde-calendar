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
 * Verbatim.
 * ----------------------------------------------------------------------------
 * 移植自 dde-calendar（src/calendar-common/src/lunarandfestival/lunarcalendar.h）。
 */
#ifndef LUNARCALENDAR_H
#define LUNARCALENDAR_H
#include "lunarandfestival.h"
#include "method_interface.h"

#include <QObject>
#include <QMap>

class LunarCalendar
{
public:
    static LunarCalendar *GetLunarCalendar(qint32 year);
    //程序退出时情况数据
    static void LogOffEmptyData();
    lunarInfo SolarDayToLunarDay(qint32 month, qint32 day);

private:
    explicit LunarCalendar(qint32 year);
    void calcProcData();
    void fillMonths();
    void calcLeapMonth();
    qint32 getSolarTermInfo(qint32 month, qint32 day) const;

public:
private:
    static QMap<int, LunarCalendar *> glYearCache;
    int Year; // 公历年份
    QVector<double> SolarTermJDs; // 相关的 25 节气 北京时间 儒略日
    QVector<QDateTime> SolarTermTimes; // 对应 SolarTermJDs 转换为 time.Time 的时间
    QVector<double> NewMoonJDs; // 相关的 15 个朔日 北京时间 儒略日
    QVector<lunarInfo> Months; // 月
    QVector<int> solarTermYearDays; // 十二节的 yearDay 列表
};

#endif // LUNARCALENDAR_H
