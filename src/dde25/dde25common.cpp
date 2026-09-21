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

#include "dde25common.h"

#include <QApplication>
#include <QDBusReply>
#include <QPalette>

namespace DDE25 {

int themeType()
{
    // dde-calendar 用 DGuiApplicationHelper::themeType()：0/1 浅色、2 深色。
    // 这里根据调色板亮度近似判断，保持 setTheMe 的取值约定不变。
    return qApp->palette().color(QPalette::Window).lightness() < 128 ? 2 : 0;
}

QColor systemActiveColor()
{
    return qApp->palette().color(QPalette::Highlight);
}

QIcon navArrowIcon(bool next, int type) {
    return QIcon(QString(":/resources/icon/%1_nav%2.svg")
        .arg(next ? QStringLiteral("next") : QStringLiteral("previous"),
            type == 2 ? QStringLiteral("_dark") : QString()));
}

QString separatorStyleSheet() {
    return QStringLiteral("background-color: rgba(0, 0, 0, 0.1);");
}

Qt::DayOfWeek fromGxdeWeekday(int weekday)
{
    // gxde-calendar: Monday = 6 … Sunday = 0
    // Qt:            Monday = 1 … Sunday = 7
    const int day = 7 - weekday;
    return static_cast<Qt::DayOfWeek>(day == 0 ? 7 : day);
}

QDate firstDayOfWeek(const QDate &date, Qt::DayOfWeek firstDay)
{
    const int offset = (date.dayOfWeek() - static_cast<int>(firstDay) + 7) % 7;
    return date.addDays(-offset);
}

int weekNumOfYear(const QDate &date, Qt::DayOfWeek firstDay)
{
    // 对齐 dde-calendar 的 CalendarManager::getWeekNumOfYear。
    const QDate lastDateInWeek = firstDayOfWeek(date, firstDay).addDays(6);
    const QDate firstDayOfYear(lastDateInWeek.year(), 1, 1);
    const QDate firstShowDayOfYear = firstDayOfWeek(firstDayOfYear, firstDay);
    const qint64 dayOfShowYear = firstShowDayOfYear.daysTo(firstDayOfYear)
                                 + lastDateInWeek.dayOfYear() - 1;
    return static_cast<int>(dayOfShowYear / 7) + 1;
}

QVector<QDate> weekDates(const QDate &date, Qt::DayOfWeek firstDay)
{
    const QDate begin = firstDayOfWeek(date, firstDay);
    QVector<QDate> days;
    days.reserve(7);
    for (int i = 0; i < 7; ++i) {
        days.append(begin.addDays(i));
    }
    return days;
}

LunarCache *LunarCache::instance()
{
    static LunarCache cache;
    return &cache;
}

bool LunarCache::isMonthLoaded(int year, int month) const
{
    return m_loadedMonths.contains(year * 100 + month);
}

bool LunarCache::ensureMonth(int year, int month)
{
    if (isMonthLoaded(year, month)) {
        return false;
    }

    // 与 calendarview.cpp 使用同一套黄历服务。
    static CalendarDBus *inter = new CalendarDBus("com.deepin.api.LunarCalendar",
                                                  "/com/deepin/api/LunarCalendar",
                                                  QDBusConnection::sessionBus(), nullptr);

    bool ok = false;
    const QDBusReply<CaLunarMonthInfo> reply = inter->GetLunarMonthCalendar(year, month, false, ok);
    if (!reply.isValid() || !ok) {
        // 服务不可用时不要标记为已加载，否则后续不会再重试
        return false;
    }

    m_loadedMonths.insert(year * 100 + month);

    QDate date(year, month, 1);
    const QList<CaLunarDayInfo> days = reply.value().mCaLunarDayInfo;
    for (const CaLunarDayInfo &dayInfo : days) {
        if (!date.isValid()) {
            break;
        }
        m_cache.insert(date, dayInfo);
        date = date.addDays(1);
    }

    return true;
}

CaLunarDayInfo LunarCache::info(const QDate &date) const
{
    return m_cache.value(date);
}

QString LunarCache::lunarText(const QDate &date) const
{
    CaLunarDayInfo dayInfo = m_cache.value(date);

    // 与 calendarview.cpp 的 getLunar() 保持一致。
    if (dayInfo.mLunarDayName == QStringLiteral("初一")) {
        dayInfo.mLunarDayName = dayInfo.mLunarMonthName;
    }

    if (dayInfo.mTerm.isEmpty()) {
        return dayInfo.mLunarDayName;
    }

    return dayInfo.mTerm;
}

void LunarCache::clear()
{
    m_cache.clear();
    m_loadedMonths.clear();
}

} // namespace DDE25
