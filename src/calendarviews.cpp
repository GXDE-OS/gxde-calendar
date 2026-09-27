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
 */

#include "calendarviews.h"
#include "dde25/dde25common.h"

#include <QPainter>
#include <QLocale>
#include <QMouseEvent>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace {
const QColor AccentColor("#2ca7f8");
const QColor WeekendColor("#ff5b38");
const QColor DefaultColor("#000000");
const QColor MutedColor("#b2b2b2");
const int MonthHeaderHeight = 22;
const int YearToolbarHeight = 66;
}

YearView::YearView(QWidget *parent) : QWidget(parent) {
    m_currentDate = QDate::currentDate();
    setCursor(Qt::PointingHandCursor);

    QWidget *toolbar = new QWidget(this);
    toolbar->setFixedHeight(YearToolbarHeight);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(10, 0, 10, 0);
    toolbarLayout->setSpacing(0);

    m_yearLabel = new QLabel(toolbar);
    QFont yearFont;
    yearFont.setWeight(QFont::Medium);
    yearFont.setPixelSize(24);
    m_yearLabel->setFont(yearFont);

    m_todayFrame = new QFrame(toolbar);
    m_todayFrame->setObjectName("YearTodayFrame");
    m_todayFrame->setFixedHeight(36);

    QHBoxLayout *todayLayout = new QHBoxLayout(m_todayFrame);
    todayLayout->setContentsMargins(0, 0, 0, 0);
    todayLayout->setSpacing(0);

    m_prevButton = new QPushButton(m_todayFrame);
    m_prevButton->setIconSize(QSize(16, 16));
    m_prevButton->setFixedSize(36, 36);
    m_prevButton->setFocusPolicy(Qt::NoFocus);
    m_prevButton->setCursor(Qt::PointingHandCursor);

    m_todayButton = new QPushButton(tr("Today"), m_todayFrame);
    m_todayButton->setFixedSize(88, 36);
    m_todayButton->setFocusPolicy(Qt::NoFocus);
    m_todayButton->setCursor(Qt::PointingHandCursor);

    m_nextButton = new QPushButton(m_todayFrame);
    m_nextButton->setIconSize(QSize(16, 16));
    m_nextButton->setFixedSize(36, 36);
    m_nextButton->setFocusPolicy(Qt::NoFocus);
    m_nextButton->setCursor(Qt::PointingHandCursor);

    todayLayout->addWidget(m_prevButton);
    todayLayout->addWidget(m_todayButton);
    todayLayout->addWidget(m_nextButton);

    toolbarLayout->addWidget(m_yearLabel);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(m_todayFrame);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(toolbar);
    mainLayout->addStretch();

    connect(m_prevButton, &QPushButton::clicked, this, [this] { switchYear(-1); });
    connect(m_nextButton, &QPushButton::clicked, this, [this] { switchYear(1); });
    connect(m_todayButton, &QPushButton::clicked, this, [this] {
        m_currentDate = QDate::currentDate();
        updateYearLabel();
        update();
    });

    updateYearLabel();
    // 年份标题、今天工具栏、翻年箭头随深浅主题初始化（运行时切换在 changeEvent 里再调）
    applyTheme();
}

void YearView::changeEvent(QEvent *event)
{
    // 跟随系统调色板（深浅主题）变化，刷新年视图配色，
    // 做法同各对话框的 setTheMe：监听 PaletteChange / ApplicationPaletteChange。
    if (event->type() == QEvent::PaletteChange
            || event->type() == QEvent::ApplicationPaletteChange) {
        applyTheme();
    }
    QWidget::changeEvent(event);
}

void YearView::applyTheme()
{
    m_themeType = DDE25::themeType();
    const bool dark = m_themeType == 2;

    // 年份标题：浅色黑字、深色白字
    m_yearLabel->setStyleSheet(
        dark ? "color: rgba(255, 255, 255, 0.9);"
             : "color: rgba(0, 0, 0, 0.8);");

    // “今天”工具栏：与标题栏按钮、视图切换器观感一致
    // （setStyleSheet 作用在子控件上，不会重入本控件的 changeEvent）
    const QString frameBg = dark ? QStringLiteral("rgba(255, 255, 255, 0.08)")
                                : QStringLiteral("white");
    const QString frameBorder = dark ? QStringLiteral("rgba(255, 255, 255, 0.12)")
                                    : QStringLiteral("rgba(0, 0, 0, 0.1)");
    const QString btnColor = dark ? QStringLiteral("rgba(255, 255, 255, 0.9)")
                                 : QStringLiteral("#000000");
    const QString hoverBg = dark ? QStringLiteral("rgba(255, 255, 255, 0.1)")
                                : QStringLiteral("rgba(0, 0, 0, 0.05)");
    m_todayFrame->setStyleSheet(
        QString("QFrame#YearTodayFrame {"
                "  background-color: %1;"
                "  border: 1px solid %2;"
                "  border-radius: 4px;"
                "}"
                "QFrame#YearTodayFrame QPushButton {"
                "  background-color: transparent;"
                "  border: none;"
                "  color: %3;"
                "  border-radius: 3px;"
                "}"
                "QFrame#YearTodayFrame QPushButton:hover {"
                "  background-color: %4;"
                "}")
            .arg(frameBg, frameBorder, btnColor, hoverBg));

    // 翻年箭头图标随深浅主题（dde25common::navArrowIcon：深色用浅色箭头、浅色用深色箭头）
    m_prevButton->setIcon(DDE25::navArrowIcon(false, m_themeType));
    m_nextButton->setIcon(DDE25::navArrowIcon(true, m_themeType));
}

void YearView::setCurrentDate(const QDate &date) {
    if (!date.isValid()) {
        return;
    }

    m_currentDate = date;
    updateYearLabel();
    update();
}

void YearView::setFirstWeekday(int weekday) {
    if (weekday < 0 || weekday > 6)
        return;
    m_firstWeekDay = weekday;
    update();
}

void YearView::switchYear(int offset) {
    m_currentDate = m_currentDate.addYears(offset);
    updateYearLabel();
    update();
}

void YearView::updateYearLabel() {
    if (m_yearLabel) {
        m_yearLabel->setText(QString::number(m_currentDate.year()));
    }
}

int YearView::firstWeekdayOffset(const QDate &firstDay) const {
    // Align w/ GXDE-Calendar's CalendarView::updateDate
    return (firstDay.dayOfWeek() + m_firstWeekDay) % 7;
}

QRect YearView::monthRect(int monthIndex) const {
    const int cols = 3;
    const int rows = 4;
    const int margin = 8;
    const int gridTop = YearToolbarHeight;
    const int gridHeight = height() - YearToolbarHeight;
    const int col = monthIndex % cols;
    const int row = monthIndex / cols;
    const int cellW = (width() - margin * 2) / cols;
    const int cellH = (gridHeight - margin * 2) / rows;
    return QRect(margin + col * cellW, gridTop + margin + row * cellH, cellW, cellH);
}

void YearView::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::transparent);

    const int year = m_currentDate.year();
    const QDate today = QDate::currentDate();

    // 日期/月份标题文字色：浅色主题黑、深色主题白（避免黑字黑底看不清）
    const QColor dayText = (m_themeType == 2) ? QColor(Qt::white) : DefaultColor;

    QFont headerFont = painter.font();
    headerFont.setPixelSize(14);
    headerFont.setBold(true);

    QFont dayFont = painter.font();
    dayFont.setPixelSize(11);

    for (int month = 1; month <= 12; ++month) {
        const QRect area = monthRect(month - 1);
        const QDate firstDay(year, month, 1);
        const int offset = firstWeekdayOffset(firstDay);
        const int daysInMonth = firstDay.daysInMonth();
        const int weekRows = (offset + daysInMonth + 6) / 7;

        // Month title
        painter.setFont(headerFont);
        painter.setPen(dayText);
        const QRect headerRect(area.x(), area.y(), area.width(),
          MonthHeaderHeight);
        painter.drawText(headerRect, Qt::AlignCenter, QLocale().toString(
            firstDay, "MMM"));

        // Date grid
        const int gridTop = area.y() + MonthHeaderHeight;
        const int gridHeight = area.height() - MonthHeaderHeight;
        const int cellW = area.width() / 7;
        const int cellH = gridHeight / 6;

        painter.setFont(dayFont);
        for (int i = 0; i < 42; ++i) {
            const int row = i / 7;
            const int col = i % 7;
            if (row >= weekRows) {
                break;
            }

            const int dayNum = i - offset + 1;
            if (dayNum < 1 || dayNum > daysInMonth) {
                continue;
            }

            const QDate date(year, month, dayNum);
            const QRect cell(area.x() + col * cellW, gridTop + row * cellH,
                cellW, cellH);

            if (date == today) {
                const int side = qMin(cellW - 2, cellH - 1);
                QRect highlight(0, 0, side, side);
                highlight.moveCenter(cell.center());
                painter.setPen(Qt::NoPen);
                painter.setBrush(AccentColor);
                const int radius = qMin(6, side / 2);
                painter.drawRoundedRect(highlight, radius, radius);
                painter.setPen(Qt::white);
            } else {
                const int dow = date.dayOfWeek();
                painter.setPen((dow == Qt::Saturday || dow == Qt::Sunday) ?
                    WeekendColor : dayText);
            }
            painter.drawText(cell, Qt::AlignCenter, QString::number(dayNum));
        }
    }
}

void YearView::mousePressEvent(QMouseEvent *event) {
    const int cols = 3;
    const int margin = 8;
    const int gridTop = YearToolbarHeight;
    const int gridHeight = height() - YearToolbarHeight;
    const int cellW = (width() - margin * 2) / cols;
    const int cellH = (gridHeight - margin * 2) / 4;

    const int col = (event->position().x() - margin) / cellW;
    const int row = (event->position().y() - gridTop - margin) / cellH;
    if (col < 0 || col >= cols || row < 0 || row >= 4) {
        return;
    }

    const int month = row * cols + col + 1;
    const QRect area = monthRect(month - 1);

    const int monthGridTop = area.y() + MonthHeaderHeight;
    const int monthGridHeight = area.height() - MonthHeaderHeight;
    const int dayCellW = area.width() / 7;
    const int dayCellH = monthGridHeight / 6;

    const int dayCol = (event->position().x() - area.x()) / dayCellW;
    const int dayRow = (event->position().y() - monthGridTop) / dayCellH;
    if (dayCol < 0 || dayCol >= 7 || dayRow < 0 || dayRow >= 6) {
        return;
    }

    const int year = m_currentDate.year();
    const QDate firstDay(year, month, 1);
    const int offset = firstWeekdayOffset(firstDay);
    const int dayNum = (dayRow * 7 + dayCol) - offset + 1;

    const QDate date(year, month, dayNum);
    if (date.isValid() && date.month() == month)
        emit dateClicked(date);
}

// Week View
WeekView::WeekView(QWidget *parent) : QWidget(parent) {
    m_currentDate = QDate::currentDate();
    setCursor(Qt::PointingHandCursor);
}

void WeekView::setCurrentDate(const QDate &date) {
    if (!date.isValid()) {
        return;
    }

    m_currentDate = date;
    update();
}

void WeekView::setFirstWeekday(int weekday) {
    if (weekday < 0 || weekday > 6) {
        return;
    }

    m_firstWeekDay = weekday;
    update();
}

QDate WeekView::weekStart() const {
    const int firstDow = 7 - m_firstWeekDay;
    const int diff = (m_currentDate.dayOfWeek() - firstDow + 7) % 7;
    return m_currentDate.addDays(-diff);
}

void WeekView::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::transparent);

    const QDate start = weekStart();
    const QDate today = QDate::currentDate();
    const int cellW = width() / 7;

    QFont headerFont = painter.font();
    headerFont.setPixelSize(14);
    QFont dayFont = painter.font();
    dayFont.setPixelSize(22);
    dayFont.setBold(true);

    for (int i = 0; i < 7; ++i) {
        const QDate date = start.addDays(i);
        const QRect cell(i * cellW, 0, cellW, height());

        // Week title
        painter.setFont(headerFont);
        const int dow = date.dayOfWeek();
        painter.setPen((dow == Qt::Saturday || dow == Qt::Sunday) ?
            WeekendColor : MutedColor);
        painter.drawText(QRect(cell.x(), 12, cellW, 24), Qt::AlignCenter,
            QLocale().dayName(date.dayOfWeek(),
                QLocale::ShortFormat));

        // Date
        const QRect dayCircle(cell.center().x() - 24, cell.center().y() - 24,
            48, 48);
        painter.setFont(dayFont);
        if (date == m_currentDate) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(AccentColor);
            painter.drawEllipse(dayCircle);
            painter.setPen(Qt::white);
        } else if (date == today) {
            painter.setPen(AccentColor);
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(dayCircle);
            painter.setPen(dow == Qt::Saturday || dow == Qt::Sunday ?
                WeekendColor : DefaultColor);
        } else {
            painter.setPen(dow == Qt::Saturday || dow == Qt::Sunday ?
                WeekendColor : DefaultColor);
        }
        painter.drawText(dayCircle, Qt::AlignCenter,
            QString::number(date.day()));
    }
}

void WeekView::mousePressEvent(QMouseEvent *event) {
    const int cellW = width() / 7;
    const int col = event->position().x() / cellW;
    if (col < 0 || col >= 7)
        return;

    emit dateClicked(weekStart().addDays(col));
}

// Day view
DayView::DayView(QWidget *parent) : QWidget(parent) {
    m_currentDate = QDate::currentDate();
}

void DayView::setCurrentDate(const QDate &date) {
    if (!date.isValid()) {
        return;
    }

    m_currentDate = date;
    update();
}

void DayView::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::transparent);

    // Week name
    QFont weekdayFont = painter.font();
    weekdayFont.setPixelSize(20);
    painter.setFont(weekdayFont);
    painter.setPen(AccentColor);
    painter.drawText(QRect(0, height() / 2 - 120, width(), 30), Qt::AlignCenter,
        QLocale().dayName(m_currentDate.dayOfWeek(),
            QLocale::LongFormat));

    // Date nbr
    QFont dayFont = painter.font();
    dayFont.setPixelSize(120);
    dayFont.setBold(true);
    painter.setFont(dayFont);
    const int dow = m_currentDate.dayOfWeek();
    painter.setPen(dow == Qt::Saturday || dow == Qt::Sunday ?
        WeekendColor : DefaultColor);
    painter.drawText(QRect(0, height() / 2 - 80, width(), 140), Qt::AlignCenter,
        QString::number(m_currentDate.day()));

    // Y/M
    QFont dateFont = painter.font();
    dateFont.setPixelSize(20);
    painter.setFont(dateFont);
    painter.setPen(MutedColor);
    painter.drawText(QRect(0, height() / 2 + 70, width(), 30), Qt::AlignCenter,
        QLocale().toString(m_currentDate, "yyyy年 MMM"));
}
