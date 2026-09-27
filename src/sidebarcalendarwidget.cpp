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
 * Original author: kirigaya <kirigaya@mkacg.com>
 * Adapted from the DDE calendar sidebar calendar widget, modified to adapt the
 * GXDE calendar.
 * ----------------------------------------------------------------------------
 * 改自 dde-calendar 的侧边栏小日历，为适配 GXDE 日历做过修改。
 */

#include "sidebarcalendarwidget.h"

#include "dde25/dde25common.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QLocale>
#include <QTextOption>

static const int DayCellSize = 28;
static const int WeekLabelWidth = 28;
static const int WeekLabelHeight = 18;

SidebarCalendarWidget::SidebarCalendarWidget(QWidget *parent) : QWidget(parent)
{
    m_prevButton = new DImageButton;
    m_prevButton->setNormalPic(":/resources/icon/previous_normal.svg");
    m_prevButton->setHoverPic(":/resources/icon/previous_hover.svg");
    m_prevButton->setPressPic(":/resources/icon/previous_press.svg");
    m_prevButton->setFocusPolicy(Qt::NoFocus);

    m_nextButton = new DImageButton;
    m_nextButton->setNormalPic(":/resources/icon/next_normal.svg");
    m_nextButton->setHoverPic(":/resources/icon/next_hover.svg");
    m_nextButton->setPressPic(":/resources/icon/next_press.svg");
    m_nextButton->setFocusPolicy(Qt::NoFocus);

    m_dateLabel = new QLabel;
    m_dateLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *headerLayout = new QHBoxLayout;
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(0);
    headerLayout->addWidget(m_prevButton, 0, Qt::AlignVCenter);
    headerLayout->addStretch();
    headerLayout->addWidget(m_dateLabel, 0, Qt::AlignCenter);
    headerLayout->addStretch();
    headerLayout->addWidget(m_nextButton, 0, Qt::AlignVCenter);

    m_weekHeaderLayout = new QHBoxLayout;
    m_weekHeaderLayout->setContentsMargins(0, 0, 0, 0);
    m_weekHeaderLayout->setSpacing(0);

    m_gridLayout = new QGridLayout;
    m_gridLayout->setContentsMargins(0, 0, 0, 0);
    m_gridLayout->setSpacing(0);
    for (int i = 0; i < 42; ++i) {
        SidebarCalendarDayButton *button = new SidebarCalendarDayButton;
        button->setFixedSize(DayCellSize, DayCellSize);
        button->setFocusPolicy(Qt::NoFocus);
        m_dayButtons.append(button);
        m_gridLayout->addWidget(button, i / 7, i % 7);
        connect(button, &QPushButton::clicked, this, [this, button] {
            // 选中态自己更新：以前是靠 dateClicked 绕一圈回主视图、再由主视图
            // setDate() 灌回来才亮起来的；现在这圈不走了（见 CalendarWindow 里的连接）
            setDate(button->date());
            emit dateClicked(button->date());
        });
    }

    QWidget *gridWidget = new QWidget;
    gridWidget->setLayout(m_gridLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 10);
    mainLayout->setSpacing(0);
    // 迷你日历固定在侧栏底部（上方留给未来的日程列表）
    mainLayout->addStretch();
    mainLayout->addLayout(headerLayout);
    mainLayout->addSpacing(4);
    mainLayout->addLayout(m_weekHeaderLayout);
    mainLayout->setAlignment(m_weekHeaderLayout, Qt::AlignHCenter);
    mainLayout->addWidget(gridWidget, 0, Qt::AlignHCenter);

    connect(m_prevButton, &DImageButton::clicked, this, &SidebarCalendarWidget::slotPreviousMonth);
    connect(m_nextButton, &DImageButton::clicked, this, &SidebarCalendarWidget::slotNextMonth);

    rebuildWeekHeader();
    setDate(QDate::currentDate());
    // 标题、周几表头、自身背景随深浅主题初始化（运行时切换在 changeEvent 里再调）
    applyTheme();
}

void SidebarCalendarWidget::changeEvent(QEvent *event)
{
    // 跟随系统调色板（深浅主题）变化刷新配色；做法同各对话框 setTheMe，
    // 监听 PaletteChange / ApplicationPaletteChange。
    if (event->type() == QEvent::PaletteChange
            || event->type() == QEvent::ApplicationPaletteChange) {
        // 重入保护：applyTheme() 里的 setStyleSheet() 会令 Qt 重设本控件调色板，
        // 进而再次派发 PaletteChange，若再次进入 applyTheme 会无限递归直至栈溢出。
        if (!m_applyingTheme) {
            m_applyingTheme = true;
            applyTheme();
            m_applyingTheme = false;
        }
    }
    QWidget::changeEvent(event);
}

void SidebarCalendarWidget::applyTheme()
{
    const bool dark = DDE25::themeType() == 2;

    // 标题（年月）文字色：浅色黑、深色白
    m_dateLabel->setStyleSheet(
        dark ? "QLabel { color: rgba(255, 255, 255, 0.9); }"
             : "QLabel { color: rgba(0, 0, 0, 0.8); }");

    // 自身背景：与侧栏容器一致，浅色近白、深色深色半透明
    setStyleSheet(
        dark ? "QWidget#SidebarCalendarWidget { background-color: rgba(255, 255, 255, 0.04); border: none; }"
             : "QWidget#SidebarCalendarWidget { background-color: #67f9f9fa; border: none; }");

    // 周几表头颜色随主题刷新
    rebuildWeekHeader();

    // 日期数字按当前主题重绘（paintEvent 里读 themeType，这里强制刷新一次）
    for (QPushButton *b : m_dayButtons) {
        b->update();
    }
}

void SidebarCalendarWidget::setFirstWeekday(int weekday)
{
    if (weekday < 0 || weekday > 6)
        return;

    m_firstWeekDay = weekday;
    rebuildWeekHeader();
    updateDateGrid();
}

void SidebarCalendarWidget::setDate(const QDate &date)
{
    if (!date.isValid())
        return;

    m_selectedDate = date;
    m_displayedDate = date;

    updateHeaderLabel();
    updateDateGrid();
}

QDate SidebarCalendarWidget::selectedDate() const
{
    return m_selectedDate;
}

void SidebarCalendarWidget::slotNextMonth()
{
    m_displayedDate = m_displayedDate.addMonths(1);
    updateHeaderLabel();
    updateDateGrid();
    emit monthChanged(m_displayedDate.year(), m_displayedDate.month());
}

void SidebarCalendarWidget::slotPreviousMonth()
{
    m_displayedDate = m_displayedDate.addMonths(-1);
    updateHeaderLabel();
    updateDateGrid();
    emit monthChanged(m_displayedDate.year(), m_displayedDate.month());
}

void SidebarCalendarWidget::rebuildWeekHeader()
{
    while (QLayoutItem *item = m_weekHeaderLayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    QLocale locale;
    for (int i = 0; i != 7; ++i) {
        // 与 WeekIndicator::setList 保持一致的排列顺序
        int d = i - m_firstWeekDay;
        if (d <= 0)
            d += 7;
        if (d > 7)
            d -= 7;

        const int weekDay = d ? d : 7;
        QString name = locale.dayName(weekDay, QLocale::NarrowFormat);
        if (name.isEmpty()) {
            name = locale.dayName(weekDay, QLocale::ShortFormat);
        }
        QLabel *label = new QLabel(name);
        label->setAlignment(Qt::AlignCenter);
        label->setFixedSize(WeekLabelWidth, WeekLabelHeight);
        // 周几表头文字色随深浅主题：浅色半透明黑、深色半透明白
        label->setStyleSheet(DDE25::themeType() == 2
            ? "QLabel { color: rgba(255, 255, 255, 0.7); }"
            : "QLabel { color: rgba(0, 0, 0, 0.5); }");

        if ((i == m_firstWeekDay - 1 && m_firstWeekDay != 0) || i == m_firstWeekDay || (m_firstWeekDay == 0 && i == 6)) {
            label->setObjectName("CalendarHeaderWeekend");
        } else {
            label->setObjectName("CalendarHeaderWeekday");
        }
        m_weekHeaderLayout->addWidget(label);
    }
}

void SidebarCalendarWidget::updateDateGrid()
{
    const QDate firstDay(m_displayedDate.year(), m_displayedDate.month(), 1);
    const int offset = (firstDay.dayOfWeek() + m_firstWeekDay) % 7;
    QDate gridDate = firstDay.addDays(-offset);

    for (int i = 0; i < 42; ++i) {
        SidebarCalendarDayButton *button = static_cast<SidebarCalendarDayButton *>(m_dayButtons.at(i));
        button->setDate(gridDate);
        button->setSelected(gridDate == m_selectedDate);
        button->setInCurrentMonth(gridDate.month() == m_displayedDate.month() && gridDate.year() == m_displayedDate.year());
        button->setToday(gridDate == QDate::currentDate());
        gridDate = gridDate.addDays(1);
    }
}

void SidebarCalendarWidget::updateHeaderLabel()
{
    m_dateLabel->setText(QLocale().toString(m_displayedDate, "MMM yyyy"));
}

SidebarCalendarDayButton::SidebarCalendarDayButton(QWidget *parent) : QPushButton(parent)
{
    setCursor(Qt::PointingHandCursor);
}

void SidebarCalendarDayButton::setDate(const QDate &date)
{
    m_date = date;
    update();
}

QDate SidebarCalendarDayButton::date() const
{
    return m_date;
}

void SidebarCalendarDayButton::setSelected(bool selected)
{
    if (m_selected != selected) {
        m_selected = selected;
        update();
    }
}

void SidebarCalendarDayButton::setInCurrentMonth(bool inCurrentMonth)
{
    if (m_inCurrentMonth != inCurrentMonth) {
        m_inCurrentMonth = inCurrentMonth;
        update();
    }
}

void SidebarCalendarDayButton::setToday(bool today)
{
    if (m_today != today) {
        m_today = today;
        update();
    }
}

void SidebarCalendarDayButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const qreal w = width();
    const qreal h = height();
    const qreal r = qMin(w, h) - 6;
    const QRectF rectf((w - r) / 2.0, (h - r) / 2.0, r, r);

    QFont font = painter.font();
    font.setPixelSize(12);
    painter.setFont(font);

    if (m_selected) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#2ca7f8"));
        painter.drawEllipse(rectf);
        painter.setPen(Qt::white);
    } else if (m_today) {
        painter.setPen(QColor("#2ca7f8"));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(rectf);
        painter.setPen(QColor("#2ca7f8"));
    } else if (!m_inCurrentMonth) {
        painter.setPen(QColor("#b2b2b2"));
    } else {
        const int dow = m_date.dayOfWeek();
        const bool dark = DDE25::themeType() == 2;
        painter.setPen((dow == Qt::Saturday || dow == Qt::Sunday)
                           ? QColor("#ff5b38")
                           : (dark ? QColor(Qt::white) : QColor("#000000")));
    }

    painter.drawText(rectf, QString::number(m_date.day()), QTextOption(Qt::AlignCenter));
    painter.end();
}
