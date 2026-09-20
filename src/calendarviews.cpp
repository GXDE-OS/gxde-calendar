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
 */

#include "calendarviews.h"

#include <QPainter>
#include <QLocale>
#include <QMouseEvent>

namespace {
const QColor AccentColor("#2ca7f8");
const QColor WeekendColor("#ff5b38");
const QColor DefaultColor("#000000");
const QColor MutedColor("#b2b2b2");
const int MonthHeaderHeight = 22;
}

YearView::YearView(QWidget *parent) : QWidget(parent) {
    m_currentDate = QDate::currentDate();
    setCursor(Qt::PointingHandCursor);
}

void YearView::setCurrentDate(const QDate &date) {
    if (!date.isValid()) {
        return;
    }

    m_currentDate = date;
    update();
}

void YearView::setFirstWeekday(int weekday) {
    if (weekday < 0 || weekday > 6)
        return;
    m_firstWeekDay = weekday;
    update();
}

int YearView::firstWeekdayOffset(const QDate &firstDay) const {
    // Align w/ GXDE-Calendar's CalendarView::updateDate
    return (firstDay.dayOfWeek() + m_firstWeekDay) % 7;
}

QRect YearView::monthRect(int monthIndex) const {
    const int cols = 3;
    const int rows = 4;
    const int margin = 8;
    const int col = monthIndex % cols;
    const int row = monthIndex / cols;
    const int cellW = (width() - margin * 2) / cols;
    const int cellH = (height() - margin * 2) / rows;
    return QRect(margin + col * cellW, margin + row * cellH, cellW, cellH);
}

void YearView::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::transparent);

    const int year = m_currentDate.year();
    const QDate today = QDate::currentDate();

    QFont headerFont = painter.font();
    headerFont.setPixelSize(14);
    headerFont.setBold(true);

    QFont dayFont = painter.font();
    dayFont.setPixelSize(12);

    for (int month = 1; month <= 12; ++month) {
        const QRect area = monthRect(month - 1);
        const QDate firstDay(year, month, 1);
        const int offset = firstWeekdayOffset(firstDay);
        const int daysInMonth = firstDay.daysInMonth();
        const int weekRows = (offset + daysInMonth + 6) / 7;

        // Month title
        painter.setFont(headerFont);
        painter.setPen(DefaultColor);
        const QRect headerRect(area.x(), area.y(), area.width(),
          MonthHeaderHeight);
        painter.drawText(headerRect, Qt::AlignCenter, QLocale().toString(
            firstDay, "MMM"));

        // Date grid
        const int gridTop = area.y() + MonthHeaderHeight;
        const int gridHeight = area.height() - MonthHeaderHeight;
        const int cellW = area.width() / 7;
        const int cellH = gridHeight / 6;

        painter.setFont(dayFont);
        for (int i = 0; i < 42; ++i) {
            const int row = i / 7;
            const int col = i % 7;
            if (row >= weekRows) {
                break;
            }

            const int dayNum = i - offset + 1;
            if (dayNum < 1 || dayNum > daysInMonth) {
                continue;
            }

            const QDate date(year, month, dayNum);
            const QRect cell(area.x() + col * cellW, gridTop + row * cellH,
                cellW, cellH);

            if (date == today) {
                painter.setPen(Qt::NoPen);
                painter.setBrush(AccentColor);
                painter.drawEllipse(cell.center().x() - cellH / 2 + 2,
                    cell.center().y() - cellH / 2 + 2, cellH - 4, cellH - 4);
                painter.setPen(Qt::white);
            } else {
                const int dow = date.dayOfWeek();
                painter.setPen((dow == Qt::Saturday || dow == Qt::Sunday) ?
                    WeekendColor : DefaultColor);
            }
            painter.drawText(cell, Qt::AlignCenter, QString::number(dayNum));
        }
    }
}

void YearView::mousePressEvent(QMouseEvent *event) {
    const int cols = 3;
    const int margin = 8;
    const int cellW = (width() - margin * 2) / cols;
    const int cellH = (height() - margin * 2) / 4;

    const int col = (event->position().x() - margin) / cellW;
    const int row = (event->position().y() - margin) / cellH;
    if (col < 0 || col >= cols || row < 0 || row >= 4) {
        return;
    }

    const int month = row * cols + col + 1;
    const QRect area = monthRect(month - 1);

    const int gridTop = area.y() + MonthHeaderHeight;
    const int gridHeight = area.height() - MonthHeaderHeight;
    const int dayCellW = area.width() / 7;
    const int dayCellH = gridHeight / 6;

    const int dayCol = (event->position().x() - area.x()) / dayCellW;
    const int dayRow = (event->position().y() - gridTop) / dayCellH;
    if (dayCol < 0 || dayCol >= 7 || dayRow < 0 || dayRow >= 6) {
        return;
    }

    const int year = m_currentDate.year();
    const QDate firstDay(year, month, 1);
    const int offset = firstWeekdayOffset(firstDay);
    const int dayNum = (dayRow * 7 + dayCol) - offset + 1;

    const QDate date(year, month, dayNum);
    if (date.isValid() && date.month() == month)
        emit dateClicked(date);
}

// Week View
WeekView::WeekView(QWidget *parent) : QWidget(parent) {
    m_currentDate = QDate::currentDate();
    setCursor(Qt::PointingHandCursor);
}

void WeekView::setCurrentDate(const QDate &date) {
    if (!date.isValid()) {
        return;
    }

    m_currentDate = date;
    update();
}

void WeekView::setFirstWeekday(int weekday) {
    if (weekday < 0 || weekday > 6) {
        return;
    }

    m_firstWeekDay = weekday;
    update();
}

QDate WeekView::weekStart() const {
    const int firstDow = 7 - m_firstWeekDay;
    const int diff = (m_currentDate.dayOfWeek() - firstDow + 7) % 7;
    return m_currentDate.addDays(-diff);
}

void WeekView::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::transparent);

    const QDate start = weekStart();
    const QDate today = QDate::currentDate();
    const int cellW = width() / 7;

    QFont headerFont = painter.font();
    headerFont.setPixelSize(14);
    QFont dayFont = painter.font();
    dayFont.setPixelSize(22);
    dayFont.setBold(true);

    for (int i = 0; i < 7; ++i) {
        const QDate date = start.addDays(i);
        const QRect cell(i * cellW, 0, cellW, height());

        // Week title
        painter.setFont(headerFont);
        const int dow = date.dayOfWeek();
        painter.setPen((dow == Qt::Saturday || dow == Qt::Sunday) ?
            WeekendColor : MutedColor);
        painter.drawText(QRect(cell.x(), 12, cellW, 24), Qt::AlignCenter,
            QLocale().dayName(date.dayOfWeek(),
                QLocale::ShortFormat));

        // Date
        const QRect dayCircle(cell.center().x() - 24, cell.center().y() - 24,
            48, 48);
        painter.setFont(dayFont);
        if (date == m_currentDate) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(AccentColor);
            painter.drawEllipse(dayCircle);
            painter.setPen(Qt::white);
        } else if (date == today) {
            painter.setPen(AccentColor);
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(dayCircle);
            painter.setPen(dow == Qt::Saturday || dow == Qt::Sunday ?
                WeekendColor : DefaultColor);
        } else {
            painter.setPen(dow == Qt::Saturday || dow == Qt::Sunday ?
                WeekendColor : DefaultColor);
        }
        painter.drawText(dayCircle, Qt::AlignCenter,
            QString::number(date.day()));
    }
}

void WeekView::mousePressEvent(QMouseEvent *event) {
    const int cellW = width() / 7;
    const int col = event->position().x() / cellW;
    if (col < 0 || col >= 7)
        return;

    emit dateClicked(weekStart().addDays(col));
}

// Day view
DayView::DayView(QWidget *parent) : QWidget(parent) {
    m_currentDate = QDate::currentDate();
}

void DayView::setCurrentDate(const QDate &date) {
    if (!date.isValid()) {
        return;
    }

    m_currentDate = date;
    update();
}

void DayView::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::transparent);

    // Week name
    QFont weekdayFont = painter.font();
    weekdayFont.setPixelSize(20);
    painter.setFont(weekdayFont);
    painter.setPen(AccentColor);
    painter.drawText(QRect(0, height() / 2 - 120, width(), 30), Qt::AlignCenter,
        QLocale().dayName(m_currentDate.dayOfWeek(),
            QLocale::LongFormat));

    // Date nbr
    QFont dayFont = painter.font();
    dayFont.setPixelSize(120);
    dayFont.setBold(true);
    painter.setFont(dayFont);
    const int dow = m_currentDate.dayOfWeek();
    painter.setPen(dow == Qt::Saturday || dow == Qt::Sunday ?
        WeekendColor : DefaultColor);
    painter.drawText(QRect(0, height() / 2 - 80, width(), 140), Qt::AlignCenter,
        QString::number(m_currentDate.day()));

    // Y/M
    QFont dateFont = painter.font();
    dateFont.setPixelSize(20);
    painter.setFont(dateFont);
    painter.setPen(MutedColor);
    painter.drawText(QRect(0, height() / 2 + 70, width(), 30), Qt::AlignCenter,
        QLocale().toString(m_currentDate, "yyyy年 MMM"));
}
