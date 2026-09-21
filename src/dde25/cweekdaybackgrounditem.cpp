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
 * 移植自 dde-calendar（src/calendar-client/src/view/graphicsItem/cweekdaybackgrounditem.*）。
 * 差异：去掉 qCDebug 日志。日程项的焦点切换逻辑保留，Stage 2 接入日程后即可用。
 */

#include "cweekdaybackgrounditem.h"

#include <QMarginsF>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

CWeekDayBackgroundItem::CWeekDayBackgroundItem(QGraphicsItem *parent)
    : CSceneBackgroundItem(CSceneBackgroundItem::OnWeekView, parent)
    , m_drawDividingLine(false)
    , m_showFocus(false)
{
}

void CWeekDayBackgroundItem::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        m_weekColor = "#00429A";
        m_weekColor.setAlphaF(0.05);
        m_dividingLineColor = QColor(0, 0, 0, 13);
    } else {
        m_weekColor = "#4F9BFF";
        m_weekColor.setAlphaF(0.1);
        m_dividingLineColor = QColor(255, 255, 255, 10);
    }
}

void CWeekDayBackgroundItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    painter->setRenderHint(QPainter::Antialiasing);

    if (m_drawDividingLine) {
        // 绘制分割线
        if (getBackgroundNum() != 6) {
            painter->setPen(Qt::SolidLine);
            painter->setPen(m_dividingLineColor);
            painter->drawLine(rect().topRight(), rect().bottomRight());
        }
        // 绘制周六周日背景
        if (m_Date.dayOfWeek() > 5) {
            painter->setBrush(m_weekColor);
            painter->setPen(Qt::NoPen);
            painter->drawRect(rect());
        }
    }

    if (m_showFocus && getItemFocus()) {
        QPen framePen;
        framePen.setWidth(2);
        framePen.setColor(getSystemActiveColor());
        painter->setPen(framePen);
        painter->setBrush(Qt::NoBrush);
        const QRectF drawrect = rect().marginsRemoved(QMarginsF(1, 1, 1, 1));
        painter->drawRect(drawrect);
    }
}

void CWeekDayBackgroundItem::updateCurrentItemShow()
{
    if (m_showItemIndex >= 0) {
        if (m_item.size() > 0) {
            m_showItemIndex = m_showItemIndex < m_item.size() ? m_showItemIndex : 0;
            m_item.at(m_showItemIndex)->setItemFocus(true);
            // 定位到当前焦点 item
            emit signalPosOnView(m_item.at(m_showItemIndex)->rect().y());
        } else {
            m_showItemIndex = -1;
            setItemFocus(true);
        }
    }
}

bool CWeekDayBackgroundItem::showFocus() const
{
    return m_showFocus;
}

void CWeekDayBackgroundItem::setShowFocus(bool showFocus)
{
    m_showFocus = showFocus;
}

void CWeekDayBackgroundItem::setItemFocus(bool isFocus)
{
    // 如果该背景不接受焦点，第一次设置进入该背景则设置该背景上第一个 item 接收 focus
    if (m_showFocus == false && m_showItemIndex < 0) {
        if (hasNextSubItem()) {
            ++m_showItemIndex;
            m_item.at(m_showItemIndex)->setItemFocus(isFocus);
        }
    } else {
        CSceneBackgroundItem::setItemFocus(isFocus);
    }
}

bool CWeekDayBackgroundItem::hasNextSubItem()
{
    if (m_showItemIndex < 0 && getShowItemCount() == 0) {
        return false;
    }
    if (m_showItemIndex == getShowItemCount() - 1) {
        return false;
    }
    return true;
}

bool CWeekDayBackgroundItem::drawDividingLine() const
{
    return m_drawDividingLine;
}

void CWeekDayBackgroundItem::setDrawDividingLine(bool drawDividingLine)
{
    m_drawDividingLine = drawDividingLine;
}
