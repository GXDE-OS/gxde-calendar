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
 * Origin copyright bearer: 2015 - 2026 UnionTech Software Technology Co., Ltd.
 * This file is ported from DDE calendar tag 6.6.0
 * Minimal modification is applied to make the class build against
 * DTK2Widget-Qt6.
 * ----------------------------------------------------------------------------
 * 移植自 dde-calendar（src/calendar-client/src/widget/monthWidget/monthview.*）。
 */

#include "monthview.h"

#include "constants.h"
#include "dde25common.h"
#include "monthgraphicsview.h"
#include "monthweekview.h"
#include "schedule/calendarservice.h"
#include "schedulequery.h"

#include <QVBoxLayout>

CMonthView::CMonthView(QWidget *parent)
    : QWidget(parent)
{
    m_currentDate = QDate::currentDate();

    m_weekIndicator = new CMonthWeekView(this);
    m_monthGraphicsView = new CMonthGraphicsview(this);

    connect(m_monthGraphicsView, &CMonthGraphicsview::signalsViewSelectDate,
            this, &CMonthView::signalsViewSelectDate);
    connect(m_monthGraphicsView, &CMonthGraphicsview::signalAngleDelta,
            this, &CMonthView::signalAngleDelta);
    connect(m_monthGraphicsView, &CMonthGraphicsview::signalCreateSchedule,
            this, &CMonthView::signalCreateSchedule);
    connect(m_monthGraphicsView, &CMonthGraphicsview::signalEditSchedule,
            this, &CMonthView::signalEditSchedule);

    // 视图自己订阅日程变化，省得 CalendarWindow/CMonthWindow 层层透传一个 setScheduleInfo。
    // 增删改都走 scheduleUpdate()，重查一遍即可。
    connect(CalendarService::instance(), &CalendarService::scheduleUpdate,
            this, &CMonthView::refresh);

    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_weekIndicator);
    m_mainLayout->addWidget(m_monthGraphicsView);

    setLayout(m_mainLayout);

    // 与 dde-calendar 一致：标题栏高度占整体的 10.42%
    m_weekIndicator->setFixedHeight(static_cast<int>(height() * 0.1042 + 0.5));
    m_weekIndicator->setFirstDay(m_firstWeekDay);
    m_weekIndicator->setCurrentDate(m_currentDate);
}

CMonthView::~CMonthView() = default;

void CMonthView::setTheMe(int type)
{
    m_weekIndicator->setTheMe(type);
    m_monthGraphicsView->setTheMe(type);
}

void CMonthView::setFirstWeekday(Qt::DayOfWeek weekday)
{
    m_firstWeekDay = weekday;
    m_weekIndicator->setFirstDay(weekday);
    refresh();
}

void CMonthView::setCurrentDate(const QDate &currentDate)
{
    if (!currentDate.isValid()) {
        return;
    }

    m_currentDate = currentDate;
    if (m_weekIndicator) {
        m_weekIndicator->setCurrentDate(currentDate);
    }
    refresh();
}

void CMonthView::setLunarVisible(bool visible)
{
    m_monthGraphicsView->setLunarVisible(visible);
}

void CMonthView::setFestival(const QMap<QDate, int> &festivalInfo)
{
    m_monthGraphicsView->setFestival(festivalInfo);
}

QVector<QDate> CMonthView::buildShowDates() const
{
    const QDate firstDay(m_currentDate.year(), m_currentDate.month(), 1);

    // 与 calendarview.cpp 的 updateDate() 同一套对齐规则：
    // 第一格是包含本月 1 号的那一周的周首日
    const int offset = (firstDay.dayOfWeek() - m_firstWeekDay + 7) % 7;
    const QDate start = firstDay.addDays(-offset);

    QVector<QDate> dates;
    dates.reserve(DDEMonthCalendar::ItemSizeOfMonthDay);
    for (int i = 0; i < DDEMonthCalendar::ItemSizeOfMonthDay; ++i) {
        dates.append(start.addDays(i));
    }
    return dates;
}

void CMonthView::refresh()
{
    const QVector<QDate> dates = buildShowDates();
    if (dates.isEmpty()) {
        return;
    }

    // 42 格可能跨 3 个月，逐个确保农历数据已缓存
    DDE25::LunarCache *cache = DDE25::LunarCache::instance();
    for (const QDate &date : dates) {
        cache->ensureMonth(date.year(), date.month());
    }

    // 先 setDate：它会按当前控件尺寸重算场景矩形，日程块要靠这个尺寸定位
    m_monthGraphicsView->setDate(dates);

    // 查询只在这里做，不进 paintEvent。42 格的量级很小，放在 GUI 线程足够。
    m_monthGraphicsView->setScheduleInfo(
        DDE25::querySchedules(dates.first(), dates.last()));
}

void CMonthView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    const QMargins margins = m_mainLayout->contentsMargins();
    m_weekIndicator->setFixedSize(width() - margins.left(),
                                  static_cast<int>(height() * 0.1042 + 0.5));
}
