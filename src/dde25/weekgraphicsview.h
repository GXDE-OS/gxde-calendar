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
 * 移植自 dde-calendar（src/calendar-client/src/view/cweekdaygraphicsview.* 与
 * graphicsview.* 的网格部分）。差异：本阶段不含日程项，只保留 7 列背景 + 时间网格，
 * 因此不继承 DragInfoGraphicsView，直接继承 QGraphicsView。
 */

#ifndef WEEKGRAPHICSVIEW_H
#define WEEKGRAPHICSVIEW_H

#include <QDate>
#include <QGraphicsView>
#include <QPen>
#include <QVector>

class CWeekDayBackgroundItem;

/**
 * @brief The CWeekGraphicsView class
 * 周/日视图的全天/非全天区域。Stage 1 只画背景与整点网格线。
 */
class CWeekGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    enum ViewPosition {
        WeekPos,
        DayPos
    };

    explicit CWeekGraphicsView(QWidget *parent = nullptr, ViewPosition viewPos = WeekPos);
    ~CWeekGraphicsView() override;

    // 设置场景尺寸与日期范围。h 为场景高度（= 24 小时的总像素高）。
    void setRange(int w, int h, QDate begindate, QDate enddate);
    void setRange(QDate begin, QDate end);
    void setTheMe(int type = 0);
    // 设置当前时间，用于绘制当前时刻线
    void setCurrentDate(const QDateTime &currentDate);
    // 内容尺寸变化后刷新网格
    void updateHeight();

signals:
    // 整点位置（视口坐标）与对应小时，供外层的左侧时间栏绘制文字
    void signalsPosHours(QVector<int> vPos, QVector<int> vHours, int currentTimeType);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void scrollContentsBy(int dx, int dy) override;

private:
    void createBackgroundItem();
    void setBackgroundDate();
    void setSceneRect(qreal x, qreal y, qreal w, qreal h);
    // 重新计算整点位置并通知外层
    void updateHourPos();

    ViewPosition m_viewPos;
    QGraphicsScene *m_Scene = nullptr;
    QVector<CWeekDayBackgroundItem *> m_backgroundItem;
    QDate m_beginDate;
    QDate m_endDate;

    QPen m_LRPen;
    bool m_LRFlag = true;
    QVector<int> m_vLRLarge; // 整点线的视口 y 坐标
    QVector<int> m_vHours;
    int m_currentTimeType = 0;
    QDateTime m_currentDateTime = QDateTime::currentDateTime();
    int m_rightmagin = 0;
    QColor m_gridLineColor = QColor(0, 0, 0, 13);
    QColor m_weekColor = "#00429A";
    QColor m_currenttimecolor = "#F74444";
    QColor m_outerBorderColor; // 外框背景色，用于遮住右侧竖线
};

#endif // WEEKGRAPHICSVIEW_H
