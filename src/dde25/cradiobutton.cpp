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

    int w = width();
    int h = height();

    if (!isChecked()) {
        // 未选中：填自己的颜色，外圈叠一个 10% 透明的黑环当作描边
        painter.setBrush(m_color);
        painter.drawEllipse(QPointF(w / 2, h / 2), w / 2 - 1, h / 2 - 1);

        QPainterPath path;
        path.addEllipse(0, 0, w, h);
        path.addEllipse(2, 2, w - 4, h - 4);
        QColor c = QColor("#000000");
        c.setAlphaF(0.1);
        painter.setBrush(c);
        painter.drawPath(path);
    } else {
        // 选中：外环用系统活动色，内圈留 3px 空隙再填自己的颜色
        QPainterPath path;
        path.addEllipse(0, 0, w, h);
        path.addEllipse(2, 2, w - 4, h - 4);
        painter.setBrush(DDE25::systemActiveColor());
        painter.drawPath(path);

        painter.setBrush(m_color);
        painter.drawEllipse(QPointF(w / 2, h / 2), w / 2 - 3, h / 2 - 3);
    }
}
