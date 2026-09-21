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
 */

#include "monthgraphicsview.h"

#include "cmonthdayitem.h"
#include "constants.h"
#include "dde25common.h"

#include <QApplication>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>

CMonthGraphicsview::CMonthGraphicsview(QWidget *parent)
    : QGraphicsView(parent)
{
    // 显示左右下角圆角
    m_leftShowRadius = true;
    m_rightShowRadius = true;

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    m_Scene = new QGraphicsScene(this);
    setScene(m_Scene);

    for (int i = 0; i < DDEMonthCalendar::ItemSizeOfMonthDay; ++i) {
        CMonthDayItem *item = new CMonthDayItem();
        item->setZValue(-1);
        if (!m_DayItem.isEmpty()) {
            // 设置对应左右和下一个的关系
            m_DayItem.last()->setNextFocusItem(item);
            m_DayItem.last()->setRightItem(item);
            item->setLeftItem(m_DayItem.last());
        }
        const int upNum = i - DDEMonthCalendar::AFewDaysOfWeek;
        if (upNum >= 0) {
            m_DayItem.at(upNum)->setDownItem(item);
            item->setUpItem(m_DayItem.at(upNum));
        }
        item->setBackgroundNum(i);
        m_DayItem.append(item);
        m_Scene->addItem(item);
    }

    updateSize();
}

CMonthGraphicsview::~CMonthGraphicsview()
{
    m_DayItem.clear();
}

void CMonthGraphicsview::setTheMe(int type)
{
    m_themetype = type;

    // 外框背景色，用于绘制左下/右下圆角的补角（取应用调色板，跟随深浅色主题）
    m_outerBorderColor = qApp->palette().color(QPalette::Active, QPalette::Window);

    for (CMonthDayItem *item : m_DayItem) {
        item->setTheMe(type);
    }
    viewport()->update();
}

void CMonthGraphicsview::setDate(const QVector<QDate> &showDate)
{
    Q_ASSERT(showDate.size() == DDEMonthCalendar::ItemSizeOfMonthDay);
    if (showDate.isEmpty()) {
        return;
    }

    const int currentMonth = showDate.at(0).day() != 1
        ? showDate.at(0).addMonths(1).month()
        : showDate.at(0).month();

    for (int i = 0; i < m_DayItem.size(); ++i) {
        m_DayItem.at(i)->setDate(showDate.at(i));
        m_DayItem.at(i)->setCurrentMonth(showDate.at(i).month() == currentMonth);
    }

    updateLunar();
    updateSize();
    scene()->update();
}

void CMonthGraphicsview::setFestival(const QMap<QDate, int> &festivalInfo)
{
    m_festivallist = festivalInfo;
    for (CMonthDayItem *item : m_DayItem) {
        item->setStatus(static_cast<CMonthDayItem::HolidayStatus>(m_festivallist[item->getDate()]));
    }
    if (scene()) {
        scene()->update();
    }
}

void CMonthGraphicsview::setLunarVisible(bool visible)
{
    CMonthDayItem::m_LunarVisible = visible;
    updateLunar();
    if (scene()) {
        scene()->update();
    }
}

void CMonthGraphicsview::updateSize()
{
    // 场景的大小和位置
    const QRectF sceneRect(0, 0, viewport()->rect().width(), viewport()->rect().height());
    m_Scene->setSceneRect(sceneRect);

    const qreal w = m_Scene->width() / DDEMonthCalendar::AFewDaysOfWeek;
    const qreal h = m_Scene->height() / DDEMonthCalendar::LinesNumOfMonth;

    for (int i = 0; i < m_DayItem.size(); ++i) {
        const int hOffset = i / DDEMonthCalendar::AFewDaysOfWeek;
        const int wOffset = i % DDEMonthCalendar::AFewDaysOfWeek;
        m_DayItem.at(i)->setRect(QRectF(w * wOffset, h * hOffset, w, h));
    }
}

void CMonthGraphicsview::updateLunar()
{
    DDE25::LunarCache *cache = DDE25::LunarCache::instance();

    for (CMonthDayItem *item : m_DayItem) {
        item->setLunar(cache->lunarText(item->getDate()));
    }
}

void CMonthGraphicsview::wheelEvent(QWheelEvent *event)
{
    // 参考实现：滚动为上下则发送信号用于翻月
    if (event->angleDelta().y() != 0) {
        emit signalAngleDelta(event->angleDelta().y());
    }
}

void CMonthGraphicsview::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    updateSize();
}

void CMonthGraphicsview::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);

    // 绘制圆角效果：用外框背景色补掉方角
    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_outerBorderColor);

    // 左下角补角
    if (m_leftShowRadius) {
        QPainterPath leftPath;
        leftPath.moveTo(0, height() - m_radius);
        leftPath.arcTo(0, height() - m_radius * 2, m_radius * 2, m_radius * 2, 180, 90);
        leftPath.lineTo(0, height());
        leftPath.lineTo(0, height() - m_radius);
        painter.drawPath(leftPath);
    }

    // 右下角补角
    if (m_rightShowRadius) {
        QPainterPath rightPath;
        rightPath.moveTo(width() - m_radius, height());
        rightPath.arcTo(width() - m_radius * 2, height() - m_radius * 2, m_radius * 2, m_radius * 2, 270, 90);
        rightPath.lineTo(width(), height());
        rightPath.lineTo(width() - m_radius, height());
        painter.drawPath(rightPath);
    }
}

void CMonthGraphicsview::mousePressEvent(QMouseEvent *event)
{
    const QPointF scenePos = mapToScene(event->pos());
    for (CMonthDayItem *item : m_DayItem) {
        if (item->rect().contains(scenePos) && item->getDate().isValid()) {
            emit signalsViewSelectDate(item->getDate());
            break;
        }
    }
    QGraphicsView::mousePressEvent(event);
}
