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
 *
 * 参考实现继承的是 DragInfoItem（带拖拽改期、跨格移动、长按等一整套交互）。
 * 本项目本阶段只做「把日程画出来」，所以改成继承已经移植好的 CFocusItem，
 * 保留圆角块、左侧色条、标题截断和 hover/focus 描边，交互留到后续阶段。
 */

#ifndef CMONTHSCHEDULEITEM_H
#define CMONTHSCHEDULEITEM_H

#include "cfocusitem.h"
#include "schedule/dschedule.h"

#include <QPoint>
#include <QRect>

class CMonthScheduleItem : public CFocusItem
{
    Q_OBJECT
public:
    explicit CMonthScheduleItem(QRect rect, QGraphicsItem *parent = nullptr);
    ~CMonthScheduleItem() override;

    void setData(const DSchedule::Ptr &info);
    DSchedule::Ptr getData() const { return m_scheduleInfo; }

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    DSchedule::Ptr m_scheduleInfo;
    bool m_hover = false;
    // 参考实现里标题的起始偏移
    QPoint m_pos {13, 5};
};

#endif // CMONTHSCHEDULEITEM_H
