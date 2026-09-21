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
 * graphicsview.* 的网格部分）。
 */

#include "weekgraphicsview.h"

#include "constants.h"
#include "cweekdaybackgrounditem.h"
#include "dde25common.h"

#include <QApplication>
#include <QPainter>
#include <QResizeEvent>
#include <QScrollBar>
#include <QtMath>

CWeekGraphicsView::CWeekGraphicsView(QWidget *parent, ViewPosition viewPos)
    : QGraphicsView(parent)
    , m_viewPos(viewPos)
    , m_Scene(new QGraphicsScene(this))
{
    setScene(m_Scene);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_LRPen.setStyle(Qt::SolidLine);
    m_LRFlag = true;

    createBackgroundItem();
    setTheMe(DDE25::themeType());
}

CWeekGraphicsView::~CWeekGraphicsView() = default;

void CWeekGraphicsView::createBackgroundItem()
{
    if (m_viewPos == DayPos) {
        CWeekDayBackgroundItem *backgroundItem = new CWeekDayBackgroundItem();
        backgroundItem->setZValue(-1);
        backgroundItem->setBackgroundNum(0);
        m_backgroundItem.append(backgroundItem);
        m_Scene->addItem(backgroundItem);
        return;
    }

    // 周视图：7 列
    for (int i = 0; i < DDEWeekCalendar::AFewDaysofWeek; ++i) {
        CWeekDayBackgroundItem *item = new CWeekDayBackgroundItem();
        item->setZValue(-1);
        if (!m_backgroundItem.isEmpty()) {
            m_backgroundItem.last()->setNextFocusItem(item);
            m_backgroundItem.last()->setRightItem(item);
            item->setLeftItem(m_backgroundItem.last());
        }
        // 设置背景直接分隔符
        item->setDrawDividingLine(true);
        item->setBackgroundNum(i);
        m_backgroundItem.append(item);
        m_Scene->addItem(item);
    }
}

void CWeekGraphicsView::setRange(int w, int h, QDate begindate, QDate enddate)
{
    if (w <= 0 || h <= 0) {
        return;
    }
    m_beginDate = begindate;
    m_endDate = enddate;
    setBackgroundDate();
    setSceneRect(0, 0, w, h);
    updateHourPos();
}

void CWeekGraphicsView::setRange(QDate begin, QDate end)
{
    m_beginDate = begin;
    m_endDate = end;
    setBackgroundDate();
    m_Scene->update();
}

void CWeekGraphicsView::setBackgroundDate()
{
    for (int i = 0; i < m_backgroundItem.size(); ++i) {
        m_backgroundItem.at(i)->setDate(m_beginDate.addDays(i));
    }
}

void CWeekGraphicsView::setSceneRect(qreal x, qreal y, qreal w, qreal h)
{
    m_Scene->setSceneRect(x, y, w, h);
    const qreal backgroundItemHeight = h;
    const qreal backgroundItemWidth = w / m_backgroundItem.size();

    for (int i = 0; i < m_backgroundItem.size(); ++i) {
        m_backgroundItem.at(i)->setDate(m_beginDate.addDays(i));
        m_backgroundItem.at(i)->setRect(x + backgroundItemWidth * i, 0,
                                        backgroundItemWidth, backgroundItemHeight);
    }
}

void CWeekGraphicsView::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        m_weekColor = "#00429A";
        m_weekColor.setAlphaF(0.05);
        m_gridLineColor = QColor(0, 0, 0, 13);
    } else if (type == 2) {
        m_weekColor = "#4F9BFF";
        m_weekColor.setAlphaF(0.1);
        m_gridLineColor = QColor(255, 255, 255, 10);
    }
    m_LRPen.setColor(m_gridLineColor);

    for (CWeekDayBackgroundItem *item : m_backgroundItem) {
        item->setTheMe(type);
    }

    // 外框背景色，用于遮住右侧竖线（跟随深浅色主题）
    m_outerBorderColor = qApp->palette().color(QPalette::Active, QPalette::Window);

    viewport()->update();
}

void CWeekGraphicsView::setCurrentDate(const QDateTime &currentDate)
{
    m_currentDateTime = currentDate;
    updateHourPos();
}

void CWeekGraphicsView::updateHeight()
{
    m_Scene->update();
    viewport()->update();
}

void CWeekGraphicsView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    updateHourPos();
}

void CWeekGraphicsView::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx, dy);
    updateHourPos();
}

void CWeekGraphicsView::updateHourPos()
{
    const int viewHeight = viewport()->height();
    if (viewHeight <= 0 || m_Scene->height() <= 0) {
        return;
    }

    m_vLRLarge.clear();
    m_vHours.clear();

    const qreal timeInterval = m_Scene->height() / 24.0;
    const QPointF leftTopRealPos = mapToScene(QPoint(0, 0));
    const QPointF leftBottomRealPos = mapToScene(QPoint(0, viewHeight));

    qreal beginpos = qFloor(leftTopRealPos.y() / timeInterval) * timeInterval;
    if (beginpos < leftTopRealPos.y()) {
        beginpos = (beginpos / timeInterval + 1) * timeInterval;
    }

    for (qreal i = beginpos; i < leftBottomRealPos.y(); i += timeInterval) {
        const QPoint point = mapFromScene(leftBottomRealPos.x(), i);
        m_vLRLarge.append(point.y());
        m_vHours.append(static_cast<int>(qFloor(i / timeInterval + 0.5)));
    }

    const bool showCurrentTime = m_beginDate.isValid() && m_endDate.isValid()
        && m_currentDateTime.date() >= m_beginDate
            && m_currentDateTime.date() <= m_endDate;

    const qreal currentTime = m_currentDateTime.time().msecsSinceStartOfDay() / 86400000.0
                              * m_Scene->height();
    if (showCurrentTime && currentTime > beginpos && currentTime < leftBottomRealPos.y()) {
        m_currentTimeType = 1;
        const QPoint point = mapFromScene(leftBottomRealPos.x(), currentTime);
        m_vLRLarge.append(point.y());
        m_vHours.append(static_cast<int>(qFloor(currentTime / timeInterval + 0.5)));
    } else {
        m_currentTimeType = 0;
    }

    emit signalsPosHours(m_vLRLarge, m_vHours, m_currentTimeType);
    m_Scene->update();
    viewport()->update();
}

void CWeekGraphicsView::paintEvent(QPaintEvent *event)
{
    // 先画背景与网格线，再交给 QGraphicsView 绘制 scene（包含各列背景项）
    QPainter painter(viewport());
    const int t_width = viewport()->width() + 2;

    if (m_LRFlag && !m_vLRLarge.isEmpty()) {
        painter.save();
        painter.setPen(m_LRPen);
        const int count = m_currentTimeType == 0 ? m_vLRLarge.size() : m_vLRLarge.size() - 1;
        for (int i = 0; i < count; ++i) {
            painter.drawLine(QPoint(0, m_vLRLarge[i] - 1), QPoint(t_width, m_vLRLarge[i] - 1));
        }
        painter.restore();

        if (m_currentTimeType == 1) {
            // 当前时刻线用高亮色画在最后一根
            painter.save();
            QPen pen = m_LRPen;
            pen.setColor(m_currenttimecolor);
            painter.setPen(pen);
            const int index = m_vLRLarge.size() - 1;
            painter.drawLine(QPoint(0, m_vLRLarge[index] - 1), QPoint(t_width, m_vLRLarge[index] - 1));
            painter.restore();
        }
    }

    painter.end();

    QGraphicsView::paintEvent(event);
}
