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
 * 移植自 dde-calendar（src/calendar-client/src/view/graphicsItem/scheduleitem.*）。
 *
 * 参考实现继承的是 DragInfoItem（带拖拽改期、长按、右键菜单、跨列移动一整套交互）。
 * 本项目本阶段只做「把日程画出来」，所以改成继承已经移植好的 CFocusItem，
 * 保留左侧色条、起始时间、多行标题截断和 hover/focus 描边，交互留到后续阶段。
 */

#ifndef SCHEDULEITEM_H
#define SCHEDULEITEM_H

#include "cfocusitem.h"
#include "schedule/dschedule.h"

#include <QDate>
#include <QFontMetrics>
#include <QRectF>
#include <QString>
#include <QStringList>

class CScheduleItem : public CFocusItem
{
    Q_OBJECT
public:
    CScheduleItem(QRectF rect, QGraphicsItem *parent = nullptr, int type = 0);
    ~CScheduleItem() override;

    // 设置显示数据
    void setData(const DSchedule::Ptr &info, QDate date, int totalNum);
    // 是否含有选中日程
    bool hasSelectSchedule(const DSchedule::Ptr &info);
    int getType() const { return m_type; }
    DSchedule::Ptr getData() const { return m_vScheduleInfo; }

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    // 根据字体大小、宽度和高度将标题切换为多行
    void splitText(QFont font, int w, int h, QString str, QStringList &listStr, QFontMetrics &fontM);
    // 绘制 item 显示效果
    void paintBackground(QPainter *painter, const QRectF &rect);

private:
    DSchedule::Ptr m_vScheduleInfo;
    // 0 为普通日程块，非 0 表示「还有更多」的占位块
    int m_type = 0;
    int m_totalNum = 0;
    // 参考实现这里跟随 CalendarManager 的时间制式切换（12/24 小时），本项目没有该信号
    QString m_timeFormat = "h:mm";
    bool m_hover = false;
};

#endif // SCHEDULEITEM_H
