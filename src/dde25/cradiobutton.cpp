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
 * 移植自 dde-calendar（src/calendar-client/src/customWidget/cradiobutton.*）。
 */

#include "cradiobutton.h"

#include "dde25common.h"
#include "uitheme.h"

#include <QPainter>
#include <QPainterPath>

CRadioButton::CRadioButton(QWidget *parent)
    : QRadioButton(parent)
{
}

void CRadioButton::setColor(const QColor &color)
{
    m_color = color;
}

QColor CRadioButton::getColor()
{
    return m_color;
}

void CRadioButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    // 反走样
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    const int w = width();
    const int h = height();

    // 外环：未选中是中性色描边，选中是主色。两种状态共用同一套环宽与内圈半径
    // （环 2px、内圈 w/2-2），同一个元素上不出现两套尺寸。
    QPainterPath ring;
    ring.addEllipse(0, 0, w, h);
    ring.addEllipse(2, 2, w - 4, h - 4);
    painter.setBrush(isChecked() ? DDE25::accentColor() : DDE25::borderColor());
    painter.drawPath(ring);

    painter.setBrush(m_color);
    painter.drawEllipse(QPointF(w / 2, h / 2), w / 2 - 2, h / 2 - 2);
}
