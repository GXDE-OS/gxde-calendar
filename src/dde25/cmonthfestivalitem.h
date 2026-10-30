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
 * GXDE 日历原创：月视图里的节日块。样式照搬 CMonthScheduleItem 的日程药丸
 * （圆角块 + 左侧色条 + 标题截断 + hover/focus 描边），只是颜色走警示色
 * （节假日语义），且不绑定 DSchedule，双击/右键不会触发编辑、删除日程。
 */

#ifndef CMONTHFESTIVALITEM_H
#define CMONTHFESTIVALITEM_H

#include "cfocusitem.h"

#include <QDate>
#include <QPoint>
#include <QRect>

class CMonthFestivalItem : public CFocusItem
{
    Q_OBJECT
public:
    explicit CMonthFestivalItem(QRect rect, QGraphicsItem *parent = nullptr);
    ~CMonthFestivalItem() override;

    // 设置节日名与所属日期
    void setData(const QString &name, const QDate &date);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    QString m_festivalName;
    bool m_hover = false;
    // 标题区的起始偏移，与参考实现（CMonthScheduleItem）保持一致
    QPoint m_pos {13, 5};
};

#endif // CMONTHFESTIVALITEM_H
