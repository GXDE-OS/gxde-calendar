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
 * 移植自 dde-calendar（src/calendar-client/src/customWidget/cweekwidget.*）。
 */

#include "cweekwidget.h"

#include "constants.h"

#include <QDate>
#include <QLocale>
#include <QPainter>

CWeekWidget::CWeekWidget(QWidget *parent)
    : QPushButton(parent)
{
    setMinimumHeight(10);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setFocusPolicy(Qt::NoFocus);
}

void CWeekWidget::setFirstDay(Qt::DayOfWeek first)
{
    m_firstDay = first;
    setAutoFirstDay(true);
    update();
}

void CWeekWidget::setAutoFirstDay(bool is)
{
    m_autoFirstDay = is;
}

void CWeekWidget::setAutoFontSizeByWindow(bool is)
{
    m_autoFontSizeByWindow = is;
}

void CWeekWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QFont font;

    if (m_autoFontSizeByWindow) {
        // 字体跟随界面大小
        const qreal w = this->width() / 7;
        const qreal h = this->height();
        qreal r = w > h ? h : w;

        if (QLocale::system().language() == QLocale::English) {
            r *= 0.8;
        }

        font.setPixelSize(int(r / 20.0 * 12));
    } else {
        font.setPixelSize(DDECalendar::FontSizeTwelve);
    }

    painter.setFont(font);

    QLocale locale;
    const qreal setp = width() / 7.0;

    // 一周首日：gxde 没有 CalendarManager，直接取 setFirstDay() 设进来的值
    const int firstDay = m_autoFirstDay ? m_firstDay : Qt::Monday;

    QStringList weekStr;
    for (auto i : { 7, 1, 2, 3, 4, 5, 6 }) {
        weekStr << locale.dayName(i, QLocale::NarrowFormat);
    }

    // 绘制周一到周日
    for (int i = Qt::Monday; i <= Qt::Sunday; ++i) {
        const int index = (firstDay + i - Qt::Monday) % Qt::Sunday;

        const QString text = weekStr[index];
        const QRectF rect((i - Qt::Monday) * setp, 0, setp, height());
        painter.drawText(rect, Qt::AlignCenter, text);
    }
}
