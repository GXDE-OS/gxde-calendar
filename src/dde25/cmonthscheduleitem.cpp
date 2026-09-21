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
 * 移植自 dde-calendar（src/calendar-client/src/view/graphicsItem/cmonthscheduleitem.*）。
 */

#include "cmonthscheduleitem.h"
#include "dde25common.h"
#include "schedulecolors.h"

#include <QFontMetrics>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>

CMonthScheduleItem::CMonthScheduleItem(QRect rect, QGraphicsItem *parent)
    : CFocusItem(parent)
{
    setRect(rect);
    setItemType(CITEM);
    setAcceptHoverEvents(true);
}

CMonthScheduleItem::~CMonthScheduleItem() = default;

void CMonthScheduleItem::setData(const DSchedule::Ptr &info)
{
    m_scheduleInfo = info;
    update();
}

void CMonthScheduleItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_hover = true;
    update();
    CFocusItem::hoverEnterEvent(event);
}

void CMonthScheduleItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_hover = false;
    update();
    CFocusItem::hoverLeaveEvent(event);
}

void CMonthScheduleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                               QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (m_scheduleInfo.isNull()) {
        return;
    }

    const QRectF r = rect();
    qreal labelwidth = r.width();
    qreal labelheight = r.height();

    const DDE25::CSchedulesColor gdColor =
        DDE25::scheduleColorByType(m_scheduleInfo->scheduleTypeID());
    QColor brushColor = gdColor.normalColor;
    QColor textcolor = DDE25::scheduleTextColor();

    if (getItemFocus()) {
        brushColor = gdColor.pressColor;
        textcolor.setAlphaF(0.4);
    } else if (m_hover) {
        brushColor = gdColor.hoverColor;
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

    // 左侧 2px 色条，用不透明的原色
    QColor sourceColor = QColor(m_scheduleInfo->accountColor());
    if (!sourceColor.isValid()) {
        sourceColor = gdColor.orginalColor;
    }
    painter->save();
    QPen sourcePen(sourceColor);
    sourcePen.setWidth(2);
    painter->setPen(sourcePen);
    painter->drawLine(QPointF(fillRect.left(), fillRect.top() + 1),
                      QPointF(fillRect.left(), fillRect.bottom() - 1));
    painter->restore();

    painter->setPen(textcolor);
    QString title = m_scheduleInfo->summary();
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
        QColor selcolor = (DDE25::themeType() == 2) ? QColor("#FFFFFF") : QColor("#000000");
        selcolor.setAlphaF(0.08);
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
