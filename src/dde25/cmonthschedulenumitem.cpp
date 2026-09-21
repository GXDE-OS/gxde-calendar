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
 * 移植自 dde-calendar（src/calendar-client/src/view/graphicsItem/cmonthschedulenumitem.*）。
 */

#include "cmonthschedulenumitem.h"

#include <QFontMetrics>
#include <QLinearGradient>
#include <QPainter>

CMonthScheduleNumItem::CMonthScheduleNumItem(QGraphicsItem *parent)
    : CFocusItem(parent)
{
    setItemType(COTHER);
}

CMonthScheduleNumItem::~CMonthScheduleNumItem() = default;

void CMonthScheduleNumItem::setColor(QColor color1, QColor color2)
{
    m_color1 = color1;
    m_color2 = color2;
}

void CMonthScheduleNumItem::setText(QColor tColor, QFont font)
{
    m_textcolor = tColor;
    m_font = font;
}

void CMonthScheduleNumItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                                  QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    const qreal labelwidth = rect().width();
    const qreal labelheight = rect().height() - 6;
    const qreal rectX = rect().x();
    const qreal rectY = rect().y();

    QLinearGradient linearGradient(0, 0, labelwidth, 0);
    linearGradient.setColorAt(0, m_color1);
    linearGradient.setColorAt(1, m_color2);

    painter->setRenderHints(QPainter::Antialiasing);
    painter->setBrush(linearGradient);
    if (getItemFocus()) {
        QPen framePen;
        framePen.setWidth(2);
        framePen.setColor(getSystemActiveColor());
        painter->setPen(framePen);
    } else {
        painter->setPen(Qt::NoPen);
    }
    painter->drawRoundedRect(rect(), rect().height() / 3, rect().height() / 3);

    painter->setFont(m_font);
    painter->setPen(m_textcolor);

    const QString str = QString(tr("%1 more")).arg(m_num) + "...";
    const QFontMetrics fm(painter->font());
    QString tStr;
    for (int i = 0; i < str.count(); i++) {
        tStr.append(str.at(i));
        if (fm.horizontalAdvance(tStr) + 5 >= labelwidth) {
            tStr.chop(2);
            break;
        }
    }
    if (tStr != str) {
        tStr = tStr + "...";
    }

    painter->drawText(QRectF(rectX, rectY, labelwidth, labelheight + 4), Qt::AlignCenter, tStr);
}
