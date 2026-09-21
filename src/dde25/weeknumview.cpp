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
 * 移植自 dde-calendar（src/calendar-client/src/widget/weekWidget/weekview.*）。
 */

#include "weeknumview.h"

#include "dde25common.h"

#include <QEvent>
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QToolButton>
#include <QWheelEvent>

CWeekNumWidget::CWeekNumWidget(QWidget *parent)
    : QWidget(parent)
{
    m_dayNumFont.setPixelSize(DDECalendar::FontSizeSixteen);
    m_dayNumFont.setWeight(QFont::Light);
    setFocusPolicy(Qt::StrongFocus);

    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    hBoxLayout->setSpacing(0);
    hBoxLayout->setContentsMargins(0, 0, 0, 0);

    for (int c = 0; c != DDEWeekCalendar::NumWeeksDisplayed; ++c) {
        QWidget *cell = new QWidget(this);
        cell->installEventFilter(this);
        hBoxLayout->addWidget(cell, Qt::AlignTop);
        m_cellList.append(cell);
    }
    setLayout(hBoxLayout);

    setTheMe(DDE25::themeType());
}

CWeekNumWidget::~CWeekNumWidget() = default;

void CWeekNumWidget::setTheMe(int type)
{
    m_themetype = type;

    if (type == 0 || type == 1) {
        m_defaultTextColor = QColor("#000000");
        m_defaultTextColor.setAlphaF(0.7);
        m_backgrounddefaultColor = Qt::white;
        m_currentDayTextColor = Qt::white;
        m_backgroundcurrentDayColor = DDE25::systemActiveColor();
        m_fillColor = "#FFFFFF";
    } else if (type == 2) {
        m_defaultTextColor = QColor("#FFFFFF");
        m_defaultTextColor.setAlphaF(0.7);
        m_backgrounddefaultColor = "#FFFFFF";
        m_backgrounddefaultColor.setAlphaF(0.05);
        m_currentDayTextColor = "#B8D3FF";
        m_backgroundcurrentDayColor = DDE25::systemActiveColor();
        m_fillColor = "#000000";
        m_fillColor.setAlphaF(0.05);
    }
    update();
}

void CWeekNumWidget::setSelectDate(const QDate date)
{
    m_selectDate = date;
    updateDate();
}

void CWeekNumWidget::setCurrent(const QDateTime &dateTime)
{
    m_currentDate = dateTime;
    update();
}

void CWeekNumWidget::setFirstWeekDay(Qt::DayOfWeek firstDay)
{
    m_firstDay = firstDay;
    update();
}

void CWeekNumWidget::updateDate()
{
    for (int i = 0; i < DDEWeekCalendar::NumWeeksDisplayed; ++i) {
        m_days[i] = m_selectDate.addDays((i - 4) * DDEWeekCalendar::AFewDaysofWeek);
        if (m_days[i] == m_selectDate) {
            m_selectedCell = i;
        }
    }
    update();
}

void CWeekNumWidget::resizeEvent(QResizeEvent *event)
{
    // 获取当前所有 cell 的宽度
    const int allCellWidth = width();
    const int w = allCellWidth / DDEWeekCalendar::NumWeeksDisplayed;
    // 最小显示的宽度
    const int minWidget = 36;
    // 默认都显示
    QVector<bool> vIndex(DDEWeekCalendar::NumWeeksDisplayed, true);

    if (w < minWidget) {
        // 计算前后需要隐藏的个数
        const int num = qRound((minWidget * DDEWeekCalendar::NumWeeksDisplayed - allCellWidth)
                               / static_cast<qreal>(minWidget) / 2.0);
        for (int i = 0; i < num && i < DDEWeekCalendar::NumWeeksDisplayed / 2; ++i) {
            vIndex[i] = false;
            vIndex[DDEWeekCalendar::NumWeeksDisplayed - 1 - i] = false;
        }
    }

    for (int i = 0; i < DDEWeekCalendar::NumWeeksDisplayed; ++i) {
        m_cellList[i]->setVisible(vIndex[i]);
    }

    QWidget::resizeEvent(event);
    update();
}

void CWeekNumWidget::focusInEvent(QFocusEvent *event)
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

void CWeekNumWidget::focusOutEvent(QFocusEvent *event)
{
    QWidget::focusOutEvent(event);
    m_isFocus = false;
    update();
}

bool CWeekNumWidget::event(QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
        if (keyEvent->key() == Qt::Key_Left) {
            emit signalBtnPrev();
        } else if (keyEvent->key() == Qt::Key_Right) {
            emit signalBtnNext();
        }
    }
    return QWidget::event(e);
}

bool CWeekNumWidget::eventFilter(QObject *o, QEvent *e)
{
    QWidget *cell = qobject_cast<QWidget *>(o);

    if (cell && m_cellList.contains(cell)) {
        if (e->type() == QEvent::Paint) {
            paintCell(cell);
        } else if (e->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
            if (mouseEvent->button() == Qt::LeftButton) {
                cellClicked(cell);
            }
        }
    }
    return false;
}

void CWeekNumWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    m_isFocus = false;
}

void CWeekNumWidget::cellClicked(QWidget *cell)
{
    const int pos = m_cellList.indexOf(cell);
    if (pos == -1) {
        return;
    }
    setSelectedCell(pos);
    update();
}

void CWeekNumWidget::setSelectedCell(int index)
{
    if (m_selectedCell == index) {
        return;
    }

    const int prevPos = m_selectedCell;
    m_selectedCell = index;

    if (prevPos >= 0 && prevPos < m_cellList.size()) {
        m_cellList.at(prevPos)->update();
    }
    m_cellList.at(index)->update();
    m_selectDate = m_days[index];
    emit signalsSelectDate(m_days[index]);
}

void CWeekNumWidget::paintCell(QWidget *cell)
{
    const int pos = m_cellList.indexOf(cell);
    if (pos < 0 || pos >= DDEWeekCalendar::NumWeeksDisplayed || !m_days[pos].isValid()) {
        return;
    }

    const QRect rect(0, 0, cell->width(), cell->height());
    // 与 dde-calendar 不同：不用 QDate::weekNumber()（跨年时会把不同周判成同一周），
    // 改为比较所在周的起始日。
    const bool isSelectDay = DDE25::firstDayOfWeek(m_days[pos], m_firstDay)
                             == DDE25::firstDayOfWeek(m_selectDate, m_firstDay);

    QPainter painter(cell);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.save();
    painter.setBrush(QBrush(m_fillColor));
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect);
    painter.restore();
    painter.setPen(Qt::SolidLine);

    const QString dayNum = QString::number(DDE25::weekNumOfYear(m_days[pos], m_firstDay));

    if (isSelectDay) {
        const QRect fillRect((cell->width() - 24) / 2, (cell->height() - 32) / 2 + 4, 24, 24);
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QBrush(m_backgroundcurrentDayColor));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(fillRect);

        if (m_isFocus) {
            // 绘制焦点获取效果
            QPen pen;
            pen.setWidth(2);
            pen.setColor(m_backgroundcurrentDayColor);
            painter.setPen(pen);
            const QRectF focusRect(fillRect.x() - 2, fillRect.y() - 2,
                                   fillRect.width() + 4, fillRect.height() + 4);
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(focusRect);
        }
        painter.restore();
        painter.setPen(m_currentDayTextColor);
        painter.setFont(m_dayNumFont);
        painter.drawText(QRect(0, 0, cell->width(), cell->height()), Qt::AlignCenter, dayNum);
    } else {
        if (m_currentDate.isValid()
                && DDE25::weekNumOfYear(m_currentDate.date(), m_firstDay)
                   == DDE25::weekNumOfYear(m_days[pos], m_firstDay)
                && m_days[pos].year() == m_currentDate.date().year()) {
            painter.setPen(m_backgroundcurrentDayColor);
        } else {
            painter.setPen(m_defaultTextColor);
        }
        painter.setFont(m_dayNumFont);
        painter.drawText(QRect(0, 0, cell->width(), cell->height()), Qt::AlignCenter, dayNum);
    }
    painter.end();
}

// ---------------------------------------------------------------- CWeekView

CWeekView::CWeekView(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    hBoxLayout->setContentsMargins(0, 0, 0, 0);
    hBoxLayout->setSpacing(0);

    // 上一周按钮
    QToolButton *prevButton = new QToolButton(this);
    prevButton->setObjectName("WeekPrevButton");
    prevButton->setAccessibleName("WeekPrevButton");
    prevButton->setAutoRaise(true);
    prevButton->setArrowType(Qt::LeftArrow);
    prevButton->setFixedSize(36, 36);
    connect(prevButton, &QToolButton::clicked, this, &CWeekView::signalBtnPrev);
    m_prevButton = prevButton;

    m_weekNumWidget = new CWeekNumWidget(this);

    // 下一周按钮
    QToolButton *nextButton = new QToolButton(this);
    nextButton->setObjectName("WeekNextButton");
    nextButton->setAccessibleName("WeekNextButton");
    nextButton->setAutoRaise(true);
    nextButton->setArrowType(Qt::RightArrow);
    nextButton->setFixedSize(36, 36);
    connect(nextButton, &QToolButton::clicked, this, &CWeekView::signalBtnNext);
    m_nextButton = nextButton;

    hBoxLayout->addWidget(m_prevButton);
    hBoxLayout->addWidget(m_weekNumWidget);
    hBoxLayout->addWidget(m_nextButton);
    setLayout(hBoxLayout);

    connect(m_weekNumWidget, &CWeekNumWidget::signalsSelectDate, this, &CWeekView::signalsSelectDate);
    connect(m_weekNumWidget, &CWeekNumWidget::signalBtnPrev, this, &CWeekView::signalBtnPrev);
    connect(m_weekNumWidget, &CWeekNumWidget::signalBtnNext, this, &CWeekView::signalBtnNext);
}

CWeekView::~CWeekView() = default;

void CWeekView::setSelectDate(const QDate date)
{
    m_weekNumWidget->setSelectDate(date);
}

void CWeekView::setCurrent(const QDateTime &dateTime)
{
    m_weekNumWidget->setCurrent(dateTime);
}

void CWeekView::setFirstWeekDay(Qt::DayOfWeek firstDay)
{
    m_weekNumWidget->setFirstWeekDay(firstDay);
}

void CWeekView::setTheMe(int type)
{
    m_weekNumWidget->setTheMe(type);
}

void CWeekView::wheelEvent(QWheelEvent *event)
{
    // 左移切换上周，右移切换下周
    if (event->angleDelta().y() > 0) {
        emit signalBtnPrev();
    } else {
        emit signalBtnNext();
    }
}
