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
 */

#include "schedulebodyview.h"

#include "constants.h"

#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QWheelEvent>

static const int HourTextWidth = 50;
static const int HourTextHeight = 20;
// 无全天日程时全天区的高度，对应 dde-calendar 里 m_topMagin = 32 的稳态
// （CAllDayEventWeekView 设 m_topMagin = 32 后 setFixedHeight(29) 并 signalUpdatePaint(29)）。
static const int AllDayBandHeight = 29;

CScheduleBodyView::CScheduleBodyView(QWidget *parent, CWeekGraphicsView::ViewPosition viewPos)
    : QFrame(parent)
    , m_viewPos(viewPos)
{
    setContentsMargins(0, 0, 0, 0);

    m_timeFont.setWeight(QFont::Normal);
    m_timeFont.setPixelSize(DDECalendar::FontSizeEleven);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(m_leftMargin, 0, 0, 0);

    // 全天事件区（Stage 1 只占位，绘制交给 paintEvent）
    m_allDayBand = new QWidget(this);
    m_allDayBand->setFixedHeight(AllDayBandHeight);
    layout->addWidget(m_allDayBand);

    m_graphicsView = new CWeekGraphicsView(this, m_viewPos);
    m_graphicsView->setMinimumHeight(300);
    connect(m_graphicsView, &CWeekGraphicsView::signalsPosHours,
            this, &CScheduleBodyView::slotPosHours);
    layout->addWidget(m_graphicsView);

    setLayout(layout);

    m_topMargin = AllDayBandHeight;
    m_allDayHeight = AllDayBandHeight;
}

void CScheduleBodyView::setViewMargin(int left, int top, int right, int bottom)
{
    Q_UNUSED(top)
    m_leftMargin = left;
    m_rightMargin = right;
    if (QLayout *l = layout()) {
        l->setContentsMargins(left, 0, 0, bottom);
    }
    m_topMargin = m_allDayHeight;
    update();
}

void CScheduleBodyView::setRange(int w, int h, QDate begin, QDate end)
{
    Q_UNUSED(h)
    if (!(w > 0)) {
        return;
    }
    m_beginDate = begin;
    m_endDate = end;
    m_graphicsView->setRange(w, scheduleViewHeight(), begin, end);
    update();
}

void CScheduleBodyView::setRange(QDate begin, QDate end)
{
    m_beginDate = begin;
    m_endDate = end;
    m_graphicsView->setRange(begin, end);
    update();
}

void CScheduleBodyView::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        m_dividingLineColor = QColor(0, 0, 0, 13);
        m_ALLDayColor = "#303030";
        m_timeColor = "#7D7D7D";
    } else if (type == 2) {
        m_dividingLineColor = QColor(255, 255, 255, 10);
        m_ALLDayColor = "#7D7D7D";
        m_timeColor = "#7D7D7D";
    }

    // 外框背景色，用于遮住右侧竖线
    m_outerBorderColor = qApp->palette().color(QPalette::Active, QPalette::Window);

    m_graphicsView->setTheMe(type);
    update();
}

void CScheduleBodyView::setTimeFormat(const QString &timeFormat)
{
    m_timeFormat = timeFormat;
    update();
}

void CScheduleBodyView::setCurrentDate(const QDateTime &currentDate)
{
    m_graphicsView->setCurrentDate(currentDate);
}

int CScheduleBodyView::scheduleViewHeight()
{
    // 与 dde-calendar 的 CScheduleView::scheduleViewHeight 保持一致
    qreal height = 24 * (0.083 * this->height() + 0.5);
    height = height < 500 ? 1035 : height;
    return qRound(height);
}

void CScheduleBodyView::slotPosHours(QVector<int> vPos, QVector<int> vHours, int currentTimeType)
{
    m_vPos = vPos;
    m_vHours = vHours;
    m_currentTimeType = currentTimeType;
    update();
}

void CScheduleBodyView::resizeEvent(QResizeEvent *event)
{
    m_graphicsView->setRange(width() - m_leftMargin, scheduleViewHeight(),
                             m_beginDate, m_endDate);
    update();
    QFrame::resizeEvent(event);
}

void CScheduleBodyView::wheelEvent(QWheelEvent *e)
{
    if (e->angleDelta().x() != 0) {
        emit signalAngleDelta(e->angleDelta().x());
    }
}

void CScheduleBodyView::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    QPainter painter(this);
    painter.setFont(m_timeFont);

    // 整点时间
    if (!m_vPos.isEmpty()) {
        painter.save();
        painter.setPen(m_timeColor);
        const int count = m_currentTimeType == 0 ? m_vPos.size() : m_vPos.size() - 1;
        for (int i = 0; i < count; ++i) {
            if (m_vHours[i] == 0 || m_vHours[i] == 24) {
                continue;
            }
            const int y = m_topMargin - 8 + m_vPos[i];
            if (y < m_topMargin) {
                continue;
            }
            painter.drawText(QRect((m_leftMargin - HourTextWidth) / 2 - 5, y,
                                   HourTextWidth, HourTextHeight),
                             Qt::AlignCenter,
                             QTime(m_vHours[i], 0).toString(m_timeFormat));
        }
        painter.restore();
    }

    // 全天
    painter.save();
    QFont alldayFont;
    alldayFont.setWeight(QFont::Medium);
    alldayFont.setPixelSize(DDECalendar::FontSizeFourteen);
    painter.setFont(alldayFont);
    painter.setPen(m_ALLDayColor);
    painter.drawText(QRect(0, 0, m_leftMargin - 2, m_topMargin - 2),
                     Qt::AlignCenter, tr("ALL DAY"));
    painter.restore();

    // 全天区与时间网格之间的分隔线
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_dividingLineColor);
    painter.drawRect(QRect(0, m_topMargin, width() - m_rightMargin, 1));
    painter.restore();

    // 参考实现只在周视图绘制（日视图右侧是侧栏，不需要遮竖线）
    if (m_viewPos == CWeekGraphicsView::WeekPos) {
        // 右侧背景色，遮住最后一列的分隔线（否则会残留一条竖线）
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_outerBorderColor);
        painter.drawRect(QRectF(width() - 1, 0, width(), height()));
        // 绘制三角遮住左上角圆角
        QPainterPath path;
        path.moveTo(0, 0);
        path.lineTo(m_radius, 0);
        path.lineTo(0, m_radius);
        path.lineTo(0, 0);
        painter.fillPath(path, palette().color(backgroundRole()));
        painter.restore();
    }
}
