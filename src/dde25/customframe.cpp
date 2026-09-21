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

void CustomFrame::setFixedSize(int w, int h)
{
    m_fixsizeflag = true;
    QFrame::setFixedSize(w, h);
}

void CustomFrame::paintEvent(QPaintEvent *e)
{
    const int labelwidth = width() - 2 * m_borderframew;
    const int labelheight = height() - 2 * m_borderframew;

    QPainter painter(this);
    const QRect fillRect(m_borderframew, m_borderframew, labelwidth, labelheight);

    if (m_bflag) {
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QBrush(m_bnormalColor));
        painter.setPen(Qt::NoPen);

        QPainterPath painterPath;
        painterPath.moveTo(m_radius, m_borderframew);
        if (m_lstate) {
            painterPath.arcTo(QRect(m_borderframew, m_borderframew, m_radius * 2, m_radius * 2), 90, 90);
        } else {
            painterPath.lineTo(m_borderframew, m_borderframew);
            painterPath.lineTo(m_borderframew, m_radius);
        }
        painterPath.lineTo(0, labelheight - m_radius);
        if (m_bstate) {
            painterPath.arcTo(QRect(m_borderframew, labelheight - m_radius * 2, m_radius * 2, m_radius * 2), 180, 90);
        } else {
            painterPath.lineTo(m_borderframew, labelheight);
            painterPath.lineTo(m_radius, labelheight);
        }
        painterPath.lineTo(labelwidth - m_radius, labelheight);
        if (m_rstate) {
            painterPath.arcTo(QRect(labelwidth - m_radius * 2, labelheight - m_radius * 2, m_radius * 2, m_radius * 2), 270, 90);
        } else {
            painterPath.lineTo(labelwidth, labelheight);
            painterPath.lineTo(labelwidth, labelheight - m_radius);
        }
        painterPath.lineTo(labelwidth, m_radius);
        if (m_tstate) {
            painterPath.arcTo(QRect(labelwidth - m_radius * 2, m_borderframew, m_radius * 2, m_radius * 2), 0, 90);
        } else {
            painterPath.lineTo(labelwidth, m_borderframew);
            painterPath.lineTo(labelwidth - m_radius, m_borderframew);
        }
        painterPath.lineTo(m_radius, m_borderframew);
        painterPath.closeSubpath();
        painter.drawPath(painterPath);
        painter.restore();
    }

    if (!m_text.isEmpty()) {
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setFont(m_font);
        painter.setPen(m_tnormalColor);
        painter.drawText(fillRect, m_textflag, m_text);
        painter.restore();
    }

    QFrame::paintEvent(e);
}
