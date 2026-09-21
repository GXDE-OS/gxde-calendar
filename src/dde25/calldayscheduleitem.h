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
 * 移植自 dde-calendar（src/calendar-client/src/view/graphicsItem/calldayscheduleitem.*）。
 *
 * 与 CScheduleItem 的区别：全天块的形状是圆角，标题只画一行（超出用 ... 截断），
 * 不画起始时间。和 CScheduleItem 一样，参考实现继承的 DragInfoItem 带拖拽改期那一套，
 * 这里换成已移植的 CFocusItem。
 */

#ifndef CALLDAYSCHEDULEITEM_H
#define CALLDAYSCHEDULEITEM_H

#include "cfocusitem.h"
#include "schedule/dschedule.h"

#include <QRectF>

class CAllDayScheduleItem : public CFocusItem
{
    Q_OBJECT
public:
    explicit CAllDayScheduleItem(QRectF rect, QGraphicsItem *parent = nullptr);
    ~CAllDayScheduleItem() override;

    void setData(const DSchedule::Ptr &info);
    DSchedule::Ptr getData() const { return m_vScheduleInfo; }
    // 是否含有选中日程
    bool hasSelectSchedule(const DSchedule::Ptr &info);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    // 绘制 item 显示效果
    void paintBackground(QPainter *painter, const QRectF &rect);

    DSchedule::Ptr m_vScheduleInfo;
    bool m_hover = false;
};

#endif // CALLDAYSCHEDULEITEM_H
