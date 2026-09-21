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
 * 移植自 dde-calendar（src/calendar-client/src/view/graphicsItem/cmonthschedulenumitem.*）。
 *
 * 月视图一格塞不下的日程用「还有 N 项」收尾。
 */

#ifndef CMONTHSCHEDULENUMITEM_H
#define CMONTHSCHEDULENUMITEM_H

#include "cfocusitem.h"

#include <QColor>
#include <QFont>

class CMonthScheduleNumItem : public CFocusItem
{
    Q_OBJECT
public:
    explicit CMonthScheduleNumItem(QGraphicsItem *parent = nullptr);
    ~CMonthScheduleNumItem() override;

    // 背景渐变的两端颜色
    void setColor(QColor color1, QColor color2);
    // 文字颜色与字体
    void setText(QColor tColor, QFont font);
    // 还剩多少项
    void setData(int num) { m_num = num; }

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

private:
    QColor m_color1;
    QColor m_color2;
    QColor m_textcolor;
    QFont m_font;
    int m_num = 0;
};

#endif // CMONTHSCHEDULENUMITEM_H
