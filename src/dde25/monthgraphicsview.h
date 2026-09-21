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
 * 移植自 dde-calendar（src/calendar-client/src/view/monthgraphiview.*）。
 *
 * Stage 1 说明：dde-calendar 的 CMonthGraphicsview 继承自 DragInfoGraphicsView
 * （拖拽建日程 / 键盘导航 / 右键菜单）。本阶段只做视觉外壳，因此直接继承
 * QGraphicsView，保留 42 格布局、农历、班休、今日圆点、圆角等绘制逻辑，
 * 拖拽与键盘处理留到后续阶段接入。
 */

#ifndef MONTHGRAPHICSVIEW_H
#define MONTHGRAPHICSVIEW_H

#include <QColor>
#include <QDate>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMap>
#include <QVector>

class CMonthDayItem;

class CMonthGraphicsview : public QGraphicsView
{
    Q_OBJECT
public:
    explicit CMonthGraphicsview(QWidget *parent = nullptr);
    ~CMonthGraphicsview() override;

    void setTheMe(int type = 0);
    // 设置 42 格对应的日期
    void setDate(const QVector<QDate> &showDate);
    // 设置班休信息
    void setFestival(const QMap<QDate, int> &festivalInfo);
    // 设置是否显示农历信息
    void setLunarVisible(bool visible);

signals:
    void signalsViewSelectDate(QDate date);
    // 滚动相对量（参考实现 CMonthGraphicsview::wheelEvent）
    void signalAngleDelta(int delta);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void updateSize();
    // 从 DDE25::LunarCache 读取当前 42 格对应的农历文本
    void updateLunar();

    QGraphicsScene *m_Scene = nullptr;
    QVector<CMonthDayItem *> m_DayItem;
    QMap<QDate, int> m_festivallist;
    int m_themetype = 0;

    // 左下/右下圆角
    qreal m_radius{16};
    bool m_leftShowRadius{false};
    bool m_rightShowRadius{false};
    QColor m_outerBorderColor;
};

#endif // MONTHGRAPHICSVIEW_H
