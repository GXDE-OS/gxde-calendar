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
 */

#ifndef CWEEKDAYBACKGROUNDITEM_H
#define CWEEKDAYBACKGROUNDITEM_H

#include "cscenebackgrounditem.h"

#include <QDate>

/**
 * @brief The CWeekDayBackgroundItem class
 * 周和日的全天/非全天背景绘制
 */
class CWeekDayBackgroundItem : public CSceneBackgroundItem
{
    Q_OBJECT
public:
    explicit CWeekDayBackgroundItem(QGraphicsItem *parent = nullptr);
    // 设置主题
    void setTheMe(int type = 0);
    // 是否绘制分割线
    bool drawDividingLine() const;
    // 设置是否绘制分割线
    void setDrawDividingLine(bool drawDividingLine);
    // 获取该背景是否焦点显示
    bool showFocus() const;
    // 设置该背景是否焦点显示
    void setShowFocus(bool showFocus);
    // 设置 item 是否获取 focus
    void setItemFocus(bool isFocus) override;
    // 在该背景上是否还有下一个需要焦点切换的 item
    bool hasNextSubItem();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    void updateCurrentItemShow() override;

signals:
    void signalPosOnView(const qreal y);

private:
    bool m_drawDividingLine;      // 是否绘制分割线
    bool m_showFocus;             // 背景是否显示焦点效果
    QColor m_weekColor;           // 周六周日背景色
    QColor m_dividingLineColor = QColor(0, 0, 0, 13);
};

#endif // CWEEKDAYBACKGROUNDITEM_H
