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
 */

#ifndef CFOCUSITEM_H
#define CFOCUSITEM_H

#include <QColor>
#include <QDate>
#include <QGraphicsRectItem>
#include <QObject>
#include <QPen>

class CFocusItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    enum CItemType {
        CBACK,
        CITEM,
        COTHER
    };

    explicit CFocusItem(QGraphicsItem *parent = nullptr);

    // 设置下一个 FocusItem
    void setNextFocusItem(CFocusItem *nextFocusItem);
    // 设置 item 是否获取 focus
    virtual void setItemFocus(bool isFocus);
    // 获取该 item 是否 focus
    bool getItemFocus() const;
    // 设置/获取 item 类型
    void setItemType(CItemType itemType);
    CItemType getItemType() const;
    // 设置下一个 item focus 状态并获取下一个 Item
    virtual CFocusItem *setNextItemFocusAndGetNextItem();
    // 获取系统活动色
    QColor getSystemActiveColor();
    // 设置/获取显示日期
    void setDate(const QDate &date);
    QDate getDate() const { return m_Date; }

protected:
    QDate m_Date;

private:
    CFocusItem *m_NextFocusItem;
    CItemType m_itemType;
    bool m_isFocus;
};

#endif // CFOCUSITEM_H
