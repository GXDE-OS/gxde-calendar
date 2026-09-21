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
 * Origin copyright bearer: 2015 - 2026 UnionTech Software Technology Co., Ltd.
 * This file is ported from DDE calendar tag 6.6.0
 * Minimal modification is applied to make the class build against
 * DTK2Widget-Qt6.
 * ----------------------------------------------------------------------------
 * 移植自 dde-calendar（src/calendar-client/src/customWidget/scheduleview.* 的绘制部分）。
 * 周视图与日视图共用同一个日程区（对应参考实现的 CScheduleView）。
 * 差异：Stage 1 不含日程数据，因此只有左侧时间栏 + 整点网格 + 全天区占位。
 */

#ifndef SCHEDULEBODYVIEW_H
#define SCHEDULEBODYVIEW_H

#include <QDate>
#include <QFrame>
#include <QVector>
#include <QWidget>

#include "weekgraphicsview.h"

/**
 * @brief The CScheduleBodyView class
 * 日程区外框：左侧整点时间栏 + 全天区 + 时间网格（CWeekGraphicsView）
 */
class CScheduleBodyView : public QFrame
{
    Q_OBJECT
public:
    explicit CScheduleBodyView(QWidget *parent = nullptr,
                               CWeekGraphicsView::ViewPosition viewPos = CWeekGraphicsView::WeekPos);

    // 左边距、全天区高度（对应 setTheMe/updatePaint 之后的 m_topMargin）、右边距、下边距
    void setViewMargin(int left, int top, int right, int bottom);
    void setRange(int w, int h, QDate begin, QDate end);
    void setRange(QDate begin, QDate end);
    void setTheMe(int type = 0);
    void setTimeFormat(const QString &timeFormat);
    // 设置当前时间，用于绘制当前时刻线
    void setCurrentDate(const QDateTime &currentDate);

signals:
    void signalAngleDelta(int delta);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *e) override;

private slots:
    void slotPosHours(QVector<int> vPos, QVector<int> vHours, int currentTimeType);

private:
    // 日程区（时间网格）应有的高度，与 dde-calendar 的算法一致
    int scheduleViewHeight();

    CWeekGraphicsView *m_graphicsView = nullptr;
    QWidget *m_allDayBand = nullptr;
    QVector<int> m_vPos;
    QVector<int> m_vHours;
    int m_leftMargin = 75;
    int m_topMargin = 29;
    int m_rightMargin = 0;
    int m_allDayHeight = 29;
    int m_currentTimeType = 0;
    QDate m_beginDate;
    QDate m_endDate;

    CWeekGraphicsView::ViewPosition m_viewPos = CWeekGraphicsView::WeekPos;

    QColor m_dividingLineColor = QColor(0, 0, 0, 13);
    QColor m_ALLDayColor = "#303030";
    QColor m_timeColor = "#7D7D7D";
    QColor m_outerBorderColor;
    QFont m_timeFont;
    const int m_radius = 8;
    QString m_timeFormat = "h:mm";
};

#endif // SCHEDULEBODYVIEW_H
