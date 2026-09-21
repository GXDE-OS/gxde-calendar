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
 * Origin copyright bearer: 2017 - 2026 UnionTech Software Technology Co., Ltd.
 * This file is ported from DDE calendar tag 6.6.0
 * Minimal modification is applied to make the class build against
 * DTK2Widget-Qt6.
 * ----------------------------------------------------------------------------
 * 移植自 dde-calendar（src/calendar-client/src/widget/monthWidget/monthweekview.*）。
 * 差异：基类由 DWidget 改为 QWidget；活动色改用 DDE25::systemActiveColor()。
 */

#include "monthweekview.h"

#include "constants.h"
#include "dde25common.h"

#include <QLocale>
#include <QPainter>
#include <QPainterPath>

CMonthWeekView::CMonthWeekView(QWidget *parent)
    : QWidget(parent)
{
    for (int i = 0; i < 7; ++i) {
        m_weekRect.append(new WeekRect());
    }
    // 自初始化主题与星期名，避免调用方漏调 setTheMe 时画成黑色
    setTheMe(DDE25::themeType());
    updateWeek();
}

CMonthWeekView::~CMonthWeekView()
{
    qDeleteAll(m_weekRect);
    m_weekRect.clear();
}

void CMonthWeekView::setFirstDay(const Qt::DayOfWeek weekday)
{
    m_firstWeek = weekday;
    updateWeek();
}

void CMonthWeekView::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        m_backgroundColor = "#E6EEF2";
    } else if (type == 2) {
        m_backgroundColor = "#82AEC1";
        m_backgroundColor.setAlphaF(0.10);
    }
    for (WeekRect *weekRect : m_weekRect) {
        weekRect->setTheMe(type);
    }
}

void CMonthWeekView::updateWeek()
{
    for (int i = 0; i < m_weekRect.size(); ++i) {
        const int weekNum = (m_firstWeek + i) % 7;
        const Qt::DayOfWeek setWeek = static_cast<Qt::DayOfWeek>(weekNum == 0 ? 7 : weekNum);
        // 如果为当前时间所在周则绘制横线
        m_weekRect.at(i)->setWeek(setWeek, setWeek == m_currentWeek);
    }
    update();
}

void CMonthWeekView::setCurrentDate(const QDate &currentDate)
{
    m_currentWeek = static_cast<Qt::DayOfWeek>(currentDate.dayOfWeek());
    updateWeek();
}

void CMonthWeekView::resizeEvent(QResizeEvent *event)
{
    const qreal weekRectWidth = width() / 7.0;
    for (int i = 0; i < m_weekRect.size(); ++i) {
        m_weekRect.at(i)->setRect(QRectF(i * weekRectWidth, 0, weekRectWidth, height()));
    }
    QWidget::resizeEvent(event);
}

void CMonthWeekView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath painterPath;
    // 从左下角开始，上边两角为圆角
    painterPath.moveTo(0, this->height());
    painterPath.lineTo(this->width(), this->height());
    painterPath.lineTo(this->width(), m_radius);
    painterPath.arcTo(QRect(this->width() - m_radius * 2, 0, m_radius * 2, m_radius * 2), 0, 90);
    painterPath.lineTo(m_radius, 0);
    painterPath.arcTo(QRect(0, 0, m_radius * 2, m_radius * 2), 90, 90);
    painter.setBrush(m_backgroundColor);
    painter.setPen(Qt::NoPen);
    painter.drawPath(painterPath);

    for (WeekRect *weekRect : m_weekRect) {
        weekRect->paintRect(painter);
    }
    painter.end();
}

WeekRect::WeekRect()
    : m_showLine(false)
{
    m_font.setWeight(QFont::Medium);
    m_font.setPixelSize(DDECalendar::FontSizeSixteen);
}

void WeekRect::setWeek(const Qt::DayOfWeek &showWeek, const bool &showLine)
{
    m_showWeek = showWeek;
    m_showLine = showLine;
    QLocale locale;
    m_weekStr = locale.dayName(m_showWeek, QLocale::ShortFormat);
}

void WeekRect::setRect(const QRectF &rectF)
{
    m_rectF = rectF;
}

void WeekRect::paintRect(QPainter &painter)
{
    painter.save();
    painter.setFont(m_font);
    if (m_showWeek > 5) {
        painter.setPen(m_activeColor);
    } else {
        painter.setPen(m_testColor);
    }
    painter.drawText(m_rectF, Qt::AlignCenter, m_weekStr);
    painter.restore();

    if (m_showLine) {
        // 当前星期下面画一条横线
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_activeColor);
        painter.drawRect(QRectF(m_rectF.x(), m_rectF.height() - m_lineHeight, m_rectF.width(), m_lineHeight));
        painter.restore();
    }
}

void WeekRect::setTheMe(int type)
{
    m_activeColor = DDE25::systemActiveColor();
    if (type == 0 || type == 1) {
        m_testColor = QColor(0, 0, 0, 128);
    } else {
        m_testColor = QColor(255, 255, 255, 128);
    }
}
