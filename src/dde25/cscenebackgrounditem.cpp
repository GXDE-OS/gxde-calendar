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
 * 移植自 dde-calendar（src/calendar-client/src/view/graphicsItem/cscenebackgrounditem.*）。
 */

#include "cscenebackgrounditem.h"

#include <QGraphicsScene>
#include <QMarginsF>

#include <algorithm>

namespace {
// 对显示的日程标签排序：从上到下、从左至右
bool compareItemData(const CFocusItem *itemfirst, const CFocusItem *itemsecond)
{
    if (itemfirst->rect() == itemsecond->rect()) {
        return false;
    }
    if (qAbs(itemfirst->rect().y() - itemsecond->rect().y()) < 0.01) {
        return itemfirst->rect().x() < itemsecond->rect().x();
    }
    return itemfirst->rect().y() < itemsecond->rect().y();
}
} // namespace

CSceneBackgroundItem::CSceneBackgroundItem(ItemOnView view, QGraphicsItem *parent)
    : CFocusItem(parent)
    , m_backgroundNum(0)
    , m_leftItem(nullptr)
    , m_rightItem(nullptr)
    , m_upItem(nullptr)
    , m_downItem(nullptr)
    , m_showItemIndex(-1)
    , m_itemOfView(view)
{
    setItemType(CBACK);
}

CFocusItem *CSceneBackgroundItem::setNextItemFocusAndGetNextItem()
{
    CFocusItem *nextFocus = this;
    if (m_showItemIndex < 0 && m_item.size() == 0) {
        // 该区域没有 item
        nextFocus = CFocusItem::setNextItemFocusAndGetNextItem();
    } else if (m_showItemIndex == m_item.size() - 1) {
        // 已切换到最后一个 item
        m_item.at(m_showItemIndex)->setItemFocus(false);
        m_showItemIndex = -1;
        nextFocus = CFocusItem::setNextItemFocusAndGetNextItem();
    } else {
        if (m_showItemIndex == -1 && getItemFocus()) {
            setItemFocus(false);
        }
        if (m_showItemIndex >= 0) {
            m_item.at(m_showItemIndex)->setItemFocus(false);
        }
        ++m_showItemIndex;
        m_item.at(m_showItemIndex)->setItemFocus(true);
    }
    return nextFocus;
}

void CSceneBackgroundItem::updateShowItem()
{
    m_item.clear();
    if (!scene()) {
        return;
    }

    // 缩小背景矩阵，防止获取到其他背景上的 item
    const QRectF offsetRect = rect().marginsRemoved(QMarginsF(1, 1, 1, 1));
    const QList<QGraphicsItem *> listItem = scene()->items(offsetRect);
    for (QGraphicsItem *graphicsItem : listItem) {
        CFocusItem *item = dynamic_cast<CFocusItem *>(graphicsItem);
        if (item != nullptr && item->getItemType() != CBACK) {
            m_item.append(item);
        }
    }

    std::sort(m_item.begin(), m_item.end(), compareItemData);
    updateCurrentItemShow();
}

int CSceneBackgroundItem::getShowItemCount()
{
    return m_item.size();
}

void CSceneBackgroundItem::setBackgroundNum(int num)
{
    m_backgroundNum = num;
}

int CSceneBackgroundItem::getBackgroundNum() const
{
    return m_backgroundNum;
}

void CSceneBackgroundItem::setItemFocus(bool isFocus)
{
    if (m_showItemIndex < 0) {
        CFocusItem::setItemFocus(isFocus);
    } else if (m_showItemIndex < m_item.size()) {
        m_item.at(m_showItemIndex)->setItemFocus(isFocus);
    }
}

void CSceneBackgroundItem::initState()
{
    if (getItemFocus()) {
        setItemFocus(false);
    }
    if (m_showItemIndex > -1 && m_showItemIndex < m_item.size()) {
        m_item.at(m_showItemIndex)->setItemFocus(false);
    }
    m_showItemIndex = -1;
}

CFocusItem *CSceneBackgroundItem::getFocusItem()
{
    if (m_showItemIndex < 0) {
        return this;
    }
    return m_item.at(m_showItemIndex);
}

CSceneBackgroundItem *CSceneBackgroundItem::getLeftItem() const
{
    return m_leftItem;
}

void CSceneBackgroundItem::setLeftItem(CSceneBackgroundItem *leftItem)
{
    m_leftItem = leftItem;
}

CSceneBackgroundItem *CSceneBackgroundItem::getRightItem() const
{
    return m_rightItem;
}

void CSceneBackgroundItem::setRightItem(CSceneBackgroundItem *rightItem)
{
    m_rightItem = rightItem;
}

CSceneBackgroundItem *CSceneBackgroundItem::getUpItem() const
{
    return m_upItem;
}

void CSceneBackgroundItem::setUpItem(CSceneBackgroundItem *upItem)
{
    m_upItem = upItem;
}

CSceneBackgroundItem *CSceneBackgroundItem::getDownItem() const
{
    return m_downItem;
}

void CSceneBackgroundItem::setDownItem(CSceneBackgroundItem *downItem)
{
    m_downItem = downItem;
}

void CSceneBackgroundItem::updateCurrentItemShow()
{
    if (m_showItemIndex >= 0) {
        if (m_item.size() > 0) {
            m_showItemIndex = m_showItemIndex < m_item.size() ? m_showItemIndex : 0;
            m_item.at(m_showItemIndex)->setItemFocus(true);
        } else {
            m_showItemIndex = -1;
            setItemFocus(true);
        }
    }
}

CSceneBackgroundItem::ItemOnView CSceneBackgroundItem::getItemOfView() const
{
    return m_itemOfView;
}
