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
 * 移植自 dde-calendar（src/calendar-client/src/view/graphicsItem/cfocusitem.*）。
 * 原实现通过 CScheduleDataManage 取系统活动色，这里改用 DDE25::systemActiveColor()。
 */

#include "cfocusitem.h"

#include "dde25common.h"

#include <QGraphicsScene>

CFocusItem::CFocusItem(QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
    , m_NextFocusItem(nullptr)
    , m_itemType(CITEM)
    , m_isFocus(false)
{
}

void CFocusItem::setNextFocusItem(CFocusItem *nextFocusItem)
{
    m_NextFocusItem = nextFocusItem;
}

void CFocusItem::setItemFocus(bool isFocus)
{
    m_isFocus = isFocus;
    if (scene()) {
        scene()->update();
    }
}

bool CFocusItem::getItemFocus() const
{
    return m_isFocus;
}

void CFocusItem::setItemType(CFocusItem::CItemType itemType)
{
    m_itemType = itemType;
}

CFocusItem::CItemType CFocusItem::getItemType() const
{
    return m_itemType;
}

CFocusItem *CFocusItem::setNextItemFocusAndGetNextItem()
{
    if (m_NextFocusItem != nullptr) {
        m_isFocus = false;
        m_NextFocusItem->setItemFocus(true);
    }
    return m_NextFocusItem;
}

QColor CFocusItem::getSystemActiveColor()
{
    return DDE25::systemActiveColor();
}

void CFocusItem::setDate(const QDate &date)
{
    m_Date = date;
}
