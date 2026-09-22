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
 * 移植自 dde-calendar（src/calendar-client/src/widget/monthWidget/monthwindow.*）。
 */

#include "monthwindow.h"

#include "constants.h"
#include "dde25common.h"
#include "monthdayview.h"
#include "monthview.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QVBoxLayout>

CMonthWindow::CMonthWindow(QWidget *parent)
    : QWidget(parent)
{
    m_currentDate = QDate::currentDate();

    // ---- 头部 ----
    QWidget *top = new QWidget(this);
    top->setFixedHeight(DDEMonthCalendar::M_YTopHeight);

    QHBoxLayout *yearTitleLayout = new QHBoxLayout;
    yearTitleLayout->setContentsMargins(0, 0, 0, 0);
    yearTitleLayout->setSpacing(0);
    yearTitleLayout->addSpacing(10);

    m_yearLabel = new QLabel(top);
    m_yearLabel->setFixedHeight(DDEMonthCalendar::M_YLabelHeight);
    QFont yearFont;
    yearFont.setWeight(QFont::Medium);
    yearFont.setPixelSize(DDECalendar::FontSizeTwentyfour);
    m_yearLabel->setFont(yearFont);
    yearTitleLayout->addWidget(m_yearLabel);

    m_yearLunarLabel = new QLabel(top);
    m_yearLunarLabel->setFixedSize(DDEMonthCalendar::M_YLunaLabelWindth,
                                   DDEMonthCalendar::M_YLunaLabelHeight);
    QFont lunarFont;
    lunarFont.setWeight(QFont::Medium);
    lunarFont.setPixelSize(DDECalendar::FontSizeFourteen);
    m_yearLunarLabel->setFont(lunarFont);
    yearTitleLayout->addSpacing(6);
    yearTitleLayout->addWidget(m_yearLunarLabel);

    // 与 dde-calendar 一致：两侧等权重的 stretch 把月份滚轮顶在标题栏正中，
    // 滚轮的宽度/高度由 resizeEvent 按窗口宽度的 0.5023 固定，不参与拉伸。
    yearTitleLayout->addStretch();

    m_monthDayView = new CMonthDayView(top);
    yearTitleLayout->addWidget(m_monthDayView, 0, Qt::AlignCenter);

    yearTitleLayout->addStretch();

    m_today = new QPushButton(tr("Today"), top);
    m_today->setObjectName("MonthWindowTodayButton");
    m_today->setFixedSize(DDEMonthCalendar::MTodayWindth, DDEMonthCalendar::MTodayHeight);
    QFont todayFont;
    todayFont.setWeight(QFont::Medium);
    todayFont.setPixelSize(DDECalendar::FontSizeFourteen);
    m_today->setFont(todayFont);
    m_today->setFocusPolicy(Qt::NoFocus);
    m_today->setCursor(Qt::PointingHandCursor);
    yearTitleLayout->addWidget(m_today, 0, Qt::AlignRight);

    top->setLayout(yearTitleLayout);

    // ---- 月视图 ----
    m_monthView = new CMonthView(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(top);
    mainLayout->addWidget(m_monthView);

    // 月份滚轮：左右滚动信号与日程网格的滚动信号都用来翻月
    connect(m_monthDayView, &CMonthDayView::signalsSelectDate, this, [this](const QDate &date) {
        setCurrentDate(date);
    });
    // 滚轮：向上滚（delta > 0）为上一个月，向下滚为下一个月。
    // 参考实现的 slotAngleDelta 是反的（delta > 0 为下一个月），这里跟周/日视图和
    // DDE15 模式的月历统一，免得同一个滚轮动作在不同视图里翻向相反。
    // 连续滚动时按累计量一次跳到位，免得每个档位都重建一遍月视图
    m_wheelStepper = std::make_unique<DDE25::WheelStepper>(
        [this](int steps) { setCurrentDate(m_currentDate.addMonths(-steps)); },
        DDE25::kWheelCooldownMs);
    const auto angleDeltaToSlide = [this](int delta) { m_wheelStepper->step(delta); };
    connect(m_monthDayView, &CMonthDayView::signalAngleDelta, this, angleDeltaToSlide);
    connect(m_monthView, &CMonthView::signalsViewSelectDate, this, &CMonthWindow::signalsSelectDate);
    connect(m_monthView, &CMonthView::signalAngleDelta, this, angleDeltaToSlide);
    connect(m_monthView, &CMonthView::signalCreateSchedule,
            this, &CMonthWindow::signalCreateSchedule);
    connect(m_monthView, &CMonthView::signalEditSchedule,
            this, &CMonthWindow::signalEditSchedule);
    connect(m_monthView, &CMonthView::signalDeleteSchedule,
            this, &CMonthWindow::signalDeleteSchedule);
    connect(m_today, &QPushButton::clicked, this, [this] {
        setCurrentDate(QDate::currentDate());
    });

    setTheMe(DDE25::themeType());
    setYearData();
    updateLunarYearLabel();
}

/**
 * @brief CMonthWindow::resizeEvent   窗口大小调整
 */
void CMonthWindow::resizeEvent(QResizeEvent *event)
{
    // 参考实现写死高 36（即 M_YLabelHeight），宽度为窗口宽度的 0.5023
    m_monthDayView->setFixedSize(qRound(width() * 0.5023 + 0.5), DDEMonthCalendar::M_YLabelHeight);
    QWidget::resizeEvent(event);
}

void CMonthWindow::keyPressEvent(QKeyEvent *event)
{
    // 左右方向键翻月（dde-calendar 的月视图没有上/下按钮，靠键盘与月份滚轮导航）。
    // 键盘连发也走节流，方向和翻页一致：← 上一个月，→ 下一个月（+1 = 滚轮向上 = 上个月）
    if (event->key() == Qt::Key_Left) {
        m_wheelStepper->nudge(1);
    } else if (event->key() == Qt::Key_Right) {
        m_wheelStepper->nudge(-1);
    } else {
        QWidget::keyPressEvent(event);
    }
}

void CMonthWindow::setCurrentDate(const QDate &date)
{
    if (!date.isValid()) {
        return;
    }

    // 日期没变就不重建：年月条翻月后 CalendarWindow 会把同一个日期再广播回来一次，
    // 挡掉这一下等于省掉一整遍月视图构建
    if (date == m_currentDate) {
        return;
    }

    m_currentDate = date;
    // 藏着的时候只记日期，等显示出来再重建：跨视图广播会把 M/W/D 三个视图都刷一遍，
    // 其中两个是隐藏的，白刷（隐藏时刷也白刷——Qt 不会给隐藏控件重绘）
    if (isVisible()) {
        refreshCurrentDate();
    } else {
        m_dateRefreshPending = true;
    }

    emit signalsCurrentDateChanged(date);
}

/**
 * @brief CMonthWindow::refreshCurrentDate  按 m_currentDate 重建各子控件
 */
void CMonthWindow::refreshCurrentDate()
{
    m_dateRefreshPending = false;
    m_monthView->setCurrentDate(m_currentDate);
    m_monthDayView->setSelectDate(m_currentDate);
    setYearData();
    updateLunarYearLabel();
}

void CMonthWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if (m_dateRefreshPending) {
        refreshCurrentDate();
    }
}

void CMonthWindow::setCurrentDateTime(const QDateTime &currentDate) {
    const bool dateRolledOver = m_currentDateTime.date() != currentDate.date();
    m_currentDateTime = currentDate;
    if (dateRolledOver) {
        m_monthView->refresh();
    }
}

void CMonthWindow::setFirstWeekday(Qt::DayOfWeek weekday)
{
    m_monthView->setFirstWeekday(weekday);
}

void CMonthWindow::setTheMe(int type)
{
    m_themetype = type;

    m_monthView->setTheMe(type);
    m_monthDayView->setTheMe(type);

    QColor yearTextColor("#000000");
    QColor lunarTextColor("#000000");
    if (type == 2) {
        yearTextColor = QColor("#FFFFFF");
        lunarTextColor = QColor("#FFFFFF");
    }
    yearTextColor.setAlphaF(0.8);
    lunarTextColor.setAlphaF(0.5);

    m_yearLabel->setStyleSheet(QString("color: rgba(%1,%2,%3,%4);")
                                   .arg(yearTextColor.red())
                                   .arg(yearTextColor.green())
                                   .arg(yearTextColor.blue())
                                   .arg(yearTextColor.alphaF()));
    m_yearLunarLabel->setStyleSheet(QString("color: rgba(%1,%2,%3,%4);")
                                        .arg(lunarTextColor.red())
                                        .arg(lunarTextColor.green())
                                        .arg(lunarTextColor.blue())
                                        .arg(lunarTextColor.alphaF()));
}

void CMonthWindow::setLunarVisible(bool visible)
{
    m_monthView->setLunarVisible(visible);
    updateLunarYearLabel();
}

void CMonthWindow::setFestival(const QMap<QDate, int> &festivalInfo)
{
    m_monthView->setFestival(festivalInfo);
}

void CMonthWindow::setYearData()
{
    // 按钮文案固定为 Today，不做「非今日时显示 Return Today」的切换
    m_yearLabel->setText(QString::number(m_currentDate.year()));
}

void CMonthWindow::updateLunarYearLabel()
{
    DDE25::LunarCache *cache = DDE25::LunarCache::instance();
    cache->ensureMonth(m_currentDate.year(), m_currentDate.month());

    const CaLunarDayInfo info = cache->info(m_currentDate);
    if (info.mGanZhiYear.isEmpty()) {
        m_yearLunarLabel->clear();
        return;
    }

    // 与 cschedulebasewidget.cpp 的格式一致：-<干支><生肖>年-
    m_yearLunarLabel->setText(QString("-%0%1年-").arg(info.mGanZhiYear).arg(info.mZodiac));
}
