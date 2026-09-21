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
 * 移植自 dde-calendar（src/calendar-client/src/widget/monthWidget/monthdayview.*）。
 */

#include "monthdayview.h"

#include "constants.h"
#include "dde25common.h"

#include <QFocusEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QWheelEvent>

namespace {
// 年份区间，超出范围不绘制（与 units.cpp 的 withinTimeFrame 一致）
bool withinTimeFrame(const QDate &date)
{
    return date.isValid()
           && date.year() >= DDECalendar::QueryEarliestYear
           && date.year() <= DDECalendar::QueryLatestYear;
}
} // namespace

CMonthDayView::CMonthDayView(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    hBoxLayout->setContentsMargins(10, 0, 10, 0);
    hBoxLayout->setSpacing(0);
    m_monthWidget = new CMonthWidget(this);
    hBoxLayout->addWidget(m_monthWidget);
    setLayout(hBoxLayout);

    connect(m_monthWidget, &CMonthWidget::signalsSelectDate, this, &CMonthDayView::signalsSelectDate);
    setSelectDate(QDate::currentDate());
    setTheMe(DDE25::themeType());
}

void CMonthDayView::setSelectDate(const QDate &date)
{
    m_selectDate = date;
    for (int i = 0; i < DDEMonthCalendar::MonthNumOfYear; ++i) {
        m_days[i] = m_selectDate.addMonths(i - 5);
    }
    m_monthWidget->setDate(m_days);
    update();
}

void CMonthDayView::setTheMe(int type)
{
    // 外框配色对齐 gxde-file-manager 的地址栏
    if (type == 0 || type == 1) {
        m_frameColor = QColor("#FFFFFF");
        m_borderColor = QColor(0, 0, 0, 0x1e);
    } else if (type == 2) {
        m_frameColor = QColor("#343434");
        m_borderColor = QColor(0, 0, 0, 0);
    }
    CMonthRect::setTheMe(type);
    update();
}

void CMonthDayView::wheelEvent(QWheelEvent *e)
{
    // 滚动为左右则发送横向相对量
    if (e->angleDelta().x() != 0) {
        emit signalAngleDelta(e->angleDelta().x());
    } else {
        emit signalAngleDelta(e->angleDelta().y());
    }
}

void CMonthDayView::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 与 gxde-file-manager 的地址栏一致：内缩 0.5px 画 1px 描边
    QPainterPath path;
    path.addRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), m_radius, m_radius);
    painter.fillPath(path, m_frameColor);
    if (m_borderColor.alpha() > 0) {
        painter.setPen(QPen(m_borderColor, 1));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);
    }
}

CMonthWidget::CMonthWidget(QWidget *parent)
    : QWidget(parent)
{
    for (int i = 0; i < DDEMonthCalendar::MonthNumOfYear; ++i) {
        m_MonthItem.append(new CMonthRect);
    }
    // 获取 Tab 焦点
    setFocusPolicy(Qt::StrongFocus);
}

CMonthWidget::~CMonthWidget()
{
    qDeleteAll(m_MonthItem);
    m_MonthItem.clear();
}

void CMonthWidget::setDate(const QDate date[12])
{
    for (int i = 0; i < DDEMonthCalendar::MonthNumOfYear; ++i) {
        m_MonthItem.at(i)->setDate(date[i]);
    }
    // 参考实现固定把中间的格子当作选中月
    CMonthRect::setSelectRect(m_MonthItem.at(5));
    update();
}

void CMonthWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    updateSize();
}

void CMonthWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (int i = 0; i < m_MonthItem.size(); ++i) {
        m_MonthItem.at(i)->paintItem(&painter, m_MonthItem.at(i)->rect(), m_isFocus);
    }
}

void CMonthWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        return;
    }
    m_isFocus = false;
    mousePress(event->pos());
}

void CMonthWidget::keyPressEvent(QKeyEvent *event)
{
    // 获取当前选择时间
    QDate selectDate = CMonthRect::getSelectRect()->getDate();
    QDate setdate = selectDate;
    switch (event->key()) {
    case Qt::Key_Left: {
        setdate = selectDate.addMonths(-1);
    } break;
    case Qt::Key_Right: {
        setdate = selectDate.addMonths(1);
    } break;
    default:
        QWidget::keyPressEvent(event);
    }
    if (selectDate != setdate) {
        updateShowDate(setdate);
        setDate(m_days);
        emit signalsSelectDate(setdate);
    }
}

void CMonthWidget::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent(event);
    switch (event->reason()) {
    case Qt::TabFocusReason:
    case Qt::BacktabFocusReason:
    case Qt::ActiveWindowFocusReason:
        m_isFocus = true;
        break;
    default:
        m_isFocus = false;
        break;
    }
    update();
}

void CMonthWidget::focusOutEvent(QFocusEvent *event)
{
    QWidget::focusOutEvent(event);
    m_isFocus = false;
    update();
}

void CMonthWidget::mousePress(const QPoint &point)
{
    const int itemindex = getMousePosItem(point);
    if (!(itemindex < 0)) {
        if (!withinTimeFrame(m_MonthItem.at(itemindex)->getDate())) {
            return;
        }
        CMonthRect::setSelectRect(m_MonthItem.at(itemindex));
        emit signalsSelectDate(m_MonthItem.at(itemindex)->getDate());
    }
    update();
}

void CMonthWidget::updateSize()
{
    // 参考实现写的是 width() / m_MonthItem.size()（整数除法，末尾会差 1~2px），
    // 这里保持意图做浮点除法，12 格正好铺满。
    const qreal w = this->width() / static_cast<qreal>(m_MonthItem.size());
    for (int i = 0; i < m_MonthItem.size(); ++i) {
        m_MonthItem.at(i)->setRect(i * w, 0, w, this->height());
    }
    update();
}

int CMonthWidget::getMousePosItem(const QPointF &pos)
{
    int res = -1;
    for (int i = 0; i < m_MonthItem.size(); ++i) {
        if (m_MonthItem.at(i)->rect().contains(pos)) {
            res = i;
            break;
        }
    }
    return res;
}

void CMonthWidget::updateShowDate(const QDate &selectDate)
{
    for (int i = 0; i < DDEMonthCalendar::MonthNumOfYear; ++i) {
        m_days[i] = selectDate.addMonths(i - 5);
    }
}

int CMonthRect::m_themetype = 0;
QColor CMonthRect::m_defaultTextColor;
QColor CMonthRect::m_currentDayTextColor;
QColor CMonthRect::m_backgroundcurrentDayColor;
QFont CMonthRect::m_dayNumFont;
CMonthRect *CMonthRect::m_SelectRect = nullptr;

void CMonthRect::setDate(const QDate &date)
{
    m_Date = date;
}

QDate CMonthRect::getDate() const
{
    return m_Date;
}

QRectF CMonthRect::rect() const
{
    return m_rect;
}

void CMonthRect::setRect(const QRectF &rect)
{
    m_rect = rect;
}

void CMonthRect::setRect(qreal x, qreal y, qreal w, qreal h)
{
    m_rect.setRect(x, y, w, h);
}

void CMonthRect::paintItem(QPainter *painter, const QRectF &rect, bool drawFocus)
{
    const QColor selectColor = DDE25::systemActiveColor();

    if (!withinTimeFrame(m_Date)) {
        return;
    }
    const bool isCurrentDay = (m_Date.month() == QDate::currentDate().month()
                               && m_Date.year() == QDate::currentDate().year());

    painter->setPen(Qt::SolidLine);

    const QString dayNum = QString::number(m_Date.month());

    if (m_SelectRect == this) {
        const QRectF fillRect((rect.width() - 36) / 2 + rect.x() + 6,
                              (rect.height() - 36) / 2 + 7 + rect.y(),
                              24,
                              24);
        painter->setBrush(QBrush(selectColor));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(fillRect);
        // 如果有焦点，绘制 tab 选中效果
        if (drawFocus) {
            QPen pen;
            pen.setWidth(2);
            pen.setColor(selectColor);
            painter->setPen(pen);
            // 在原有的选中效果外面再绘制一圈
            const QRectF focusRect(fillRect.x() - 2, fillRect.y() - 2,
                                   fillRect.width() + 4, fillRect.height() + 4);
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(focusRect);
        }
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(m_currentDayTextColor);
        painter->setFont(m_dayNumFont);
        painter->drawText(rect, Qt::AlignCenter, dayNum);
    } else {
        painter->setPen(isCurrentDay ? m_backgroundcurrentDayColor : m_defaultTextColor);
        painter->setFont(m_dayNumFont);
        painter->drawText(rect, Qt::AlignCenter, dayNum);
    }
}

void CMonthRect::setTheMe(int type)
{
    m_themetype = type;
    if (type == 0 || type == 1) {
        m_defaultTextColor = QColor("#000000");
        m_defaultTextColor.setAlphaF(0.7);
        m_currentDayTextColor = Qt::white;
        m_backgroundcurrentDayColor = DDE25::systemActiveColor();
    } else if (type == 2) {
        m_defaultTextColor = QColor("#FFFFFF");
        m_defaultTextColor.setAlphaF(0.7);
        m_currentDayTextColor = "#C0C6D4";
        m_backgroundcurrentDayColor = DDE25::systemActiveColor();
    }
    m_dayNumFont.setPixelSize(DDECalendar::FontSizeSixteen);
    m_dayNumFont.setWeight(QFont::Light);
}

void CMonthRect::setSelectRect(CMonthRect *selectRect)
{
    m_SelectRect = selectRect;
}

CMonthRect *CMonthRect::getSelectRect()
{
    return m_SelectRect;
}
