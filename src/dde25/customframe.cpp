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
 * 移植自 dde-calendar（src/calendar-client/src/customWidget/customframe.*）。
 */

#include "customframe.h"

#include "constants.h"

#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>

CustomFrame::CustomFrame(QWidget *parent)
    : QFrame(parent)
{
    m_font.setWeight(QFont::Medium);
    m_font.setPixelSize(DDECalendar::FontSizeFourteen);
    setAttribute(Qt::WA_TranslucentBackground);
    setContentsMargins(0, 0, 0, 0);
}

void CustomFrame::setBColor(QColor normalC)
{
    m_bnormalColor = normalC;
    m_bflag = true;
    update();
}

void CustomFrame::setRoundState(bool lstate, bool tstate, bool rstate, bool bstate)
{
    m_lstate = lstate;
    m_tstate = tstate;
    m_rstate = rstate;
    m_bstate = bstate;
}

void CustomFrame::setTextStr(const QFont &font, const QColor &tc, const QString &strc, int flag)
{
    m_font = font;
    m_tnormalColor = tc;
    m_text = strc;
    m_textflag = flag;
}

void CustomFrame::setTextStr(const QString &strc)
{
    m_text = strc;

    if (!m_fixsizeflag) {
        QFontMetrics fm(m_font);
        setMinimumWidth(fm.horizontalAdvance(m_text));
    }
    update();
}

void CustomFrame::setTextColor(QColor tc)
{
    m_tnormalColor = tc;
    update();
}

void CustomFrame::setTextFont(const QFont &font)
{
    m_font = font;

    if (!m_fixsizeflag) {
        QFontMetrics fm(m_font);
        setMinimumWidth(fm.horizontalAdvance(m_text));
    }
}

void CustomFrame::setTextAlign(int flag)
{
    m_textflag = flag;
}

void CustomFrame::setRadius(int radius)
{
    m_radius = radius;
}

void CustomFrame::setboreder(int framew)
{
    m_borderframew = framew;
}

void CustomFrame::setBorderColor(QColor borderC) {
    m_borderColor = borderC;
    update();
}

QPainterPath CustomFrame::roundedPath(const QRectF &rect) const {
    const qreal l = rect.left();
    const qreal t = rect.top();
    const qreal r = rect.right();
    const qreal b = rect.bottom();

    QPainterPath painterPath;
    painterPath.moveTo(l + m_radius, t);
    if (m_lstate) {
        painterPath.arcTo(QRectF(l, t, m_radius * 2, m_radius * 2), 90, 90);
    } else {
        painterPath.lineTo(l, t);
        painterPath.lineTo(l, t + m_radius);
    }
    painterPath.lineTo(l, b - m_radius);
    if (m_bstate) {
        painterPath.arcTo(QRectF(l, b - m_radius * 2, m_radius * 2, m_radius * 2), 180, 90);
    } else {
        painterPath.lineTo(l, b);
        painterPath.lineTo(l + m_radius, b);
    }
    painterPath.lineTo(r - m_radius, b);
    if (m_rstate) {
        painterPath.arcTo(QRectF(r - m_radius * 2, b - m_radius * 2, m_radius * 2, m_radius * 2), 270, 90);
    } else {
        painterPath.lineTo(r, b);
        painterPath.lineTo(r, b - m_radius);
    }
    painterPath.lineTo(r, t + m_radius);
    if (m_tstate) {
        painterPath.arcTo(QRectF(r - m_radius * 2, t, m_radius * 2, m_radius * 2), 0, 90);
    } else {
        painterPath.lineTo(r, t);
        painterPath.lineTo(r - m_radius, t);
    }
    painterPath.lineTo(l + m_radius, t);
    painterPath.closeSubpath();
    return painterPath;
}

void CustomFrame::setFixedSize(int w, int h)
{
    m_fixsizeflag = true;
    QFrame::setFixedSize(w, h);
}

void CustomFrame::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRectF content = QRectF(rect()).adjusted(m_borderframew, m_borderframew,
                                                   -m_borderframew, -m_borderframew);

    if (m_bflag) {
        painter.save();
        painter.setBrush(QBrush(m_bnormalColor));
        painter.setPen(Qt::NoPen);
        painter.drawPath(roundedPath(content));
        painter.restore();
    }

    if (m_borderColor.alpha() > 0) {
        painter.save();
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(m_borderColor, 1));
        painter.drawPath(roundedPath(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5)));
        painter.restore();
    }

    if (!m_text.isEmpty()) {
        painter.save();
        painter.setFont(m_font);
        painter.setPen(m_tnormalColor);
        painter.drawText(content, m_textflag, m_text);
        painter.restore();
    }

    QFrame::paintEvent(e);
}
