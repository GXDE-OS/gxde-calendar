/*
 * Copyright (C) 2026 CharOfString <root@charofstring.cc>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not see <http://www.gnu.org/licenses/>.
 * ----------------------------------------------------------------------------
 * GXDE 日历原创：月视图里的节日块。画法见头文件说明。
 */

#include "cmonthfestivalitem.h"

#include "dde25common.h"
#include "uitheme.h"

#include <QFontMetrics>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>

CMonthFestivalItem::CMonthFestivalItem(QRect rect, QGraphicsItem *parent)
    : CFocusItem(parent)
{
    setRect(rect);
    setItemType(CITEM);
    setAcceptHoverEvents(true);
}

CMonthFestivalItem::~CMonthFestivalItem() = default;

void CMonthFestivalItem::setData(const QString &name, const QDate &date)
{
    m_festivalName = name;
    setDate(date);
    // 悬浮提示显示完整节日名（块内文字可能被省略）
    setToolTip(name);
    update();
}

void CMonthFestivalItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_hover = true;
    update();
    CFocusItem::hoverEnterEvent(event);
}

void CMonthFestivalItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_hover = false;
    update();
    CFocusItem::hoverLeaveEvent(event);
}

void CMonthFestivalItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                               QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (m_festivalName.isEmpty()) {
        return;
    }

    const QRectF r = rect();
    qreal labelwidth = r.width();
    qreal labelheight = r.height();

    // 节日用警示色（节假日语义），以透明度派生出填充/悬浮/按下三档，不引入新色相
    QColor brushColor = DDE25::alertColor();
    QColor textcolor = DDE25::alertColor();

    if (getItemFocus()) {
        brushColor.setAlphaF(0.3);
        textcolor.setAlphaF(0.4);
    } else if (m_hover) {
        brushColor.setAlphaF(0.25);
    } else {
        brushColor.setAlphaF(0.15);
    }

    QFontMetrics fm(painter->font());
    // 行高比块还高时把块撑开，否则字会被压扁
    if (fm.height() > labelheight) {
        labelheight = fm.height() + 2;
    }

    const QRectF fillRect(r.x() + 2, r.y() + 2, labelwidth - 2, labelheight - 2);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);
    painter->setBrush(brushColor);
    if (getItemFocus()) {
        QPen framePen;
        framePen.setWidth(2);
        framePen.setColor(getSystemActiveColor());
        painter->setPen(framePen);
    } else {
        painter->setPen(Qt::NoPen);
    }
    painter->drawRoundedRect(fillRect, r.height() / 3, r.height() / 3);
    painter->restore();

    // 左侧 2px 色条，用不透明的警示色
    painter->save();
    QPen sourcePen(DDE25::alertColor());
    sourcePen.setWidth(2);
    painter->setPen(sourcePen);
    painter->drawLine(QPointF(fillRect.left(), fillRect.top() + 1),
                      QPointF(fillRect.left(), fillRect.bottom() - 1));
    painter->restore();

    painter->setPen(textcolor);
    QString title = m_festivalName;
    title.replace("\n", "");
    // 标题区：左侧让开 m_pos.x()，右侧再留 8px
    const qreal textWidth = labelwidth - m_pos.x() - 8;
    const QString shown = fm.elidedText(title, Qt::ElideRight, static_cast<int>(textWidth));
    painter->drawText(QRectF(r.x() + m_pos.x(), r.y() + 1, textWidth, labelheight - m_pos.y() + 3),
                      Qt::AlignLeft | Qt::AlignVCenter, shown);

    if (m_hover && !getItemFocus()) {
        QRectF tRect(r.x() + 2.5, r.y() + 2.5, labelwidth - 3, labelheight - 3);
        painter->save();
        painter->setRenderHints(QPainter::Antialiasing);
        const QColor selcolor = DDE25::hoverColor();
        QPen pen;
        pen.setColor(selcolor);
        pen.setWidthF(1);
        pen.setStyle(Qt::SolidLine);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(pen);
        painter->drawRoundedRect(tRect, r.height() / 3, r.height() / 3);
        painter->restore();
    }
}
