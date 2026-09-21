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
 * 移植自 dde-calendar（src/calendar-client/src/widget/dayWidget/daywindow.*）。
 */

#include "daywindow.h"

#include "constants.h"
#include "customframe.h"
#include "daymonthview.h"
#include "dde25common.h"
#include "schedulebodyview.h"
#include "weekgraphicsview.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QLocale>
#include <QResizeEvent>
#include <QVBoxLayout>

CDayWindow::CDayWindow(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    initConnection();
    setTheMe(DDE25::themeType());
    setCurrentDate(QDate::currentDate());
    setLunarVisible(true);
}

CDayWindow::~CDayWindow() = default;

void CDayWindow::setTheMe(int type)
{
    m_themetype = type;
    if (type == 0 || type == 1) {
        m_leftground->setBColor("#FFFFFF");

        QPalette ypa = m_YearLabel->palette();
        QColor yearTextColor = QColor("#000000");
        yearTextColor.setAlphaF(0.8);
        ypa.setColor(QPalette::WindowText, yearTextColor);
        m_YearLabel->setPalette(ypa);
        m_YearLabel->setForegroundRole(QPalette::WindowText);

        QPalette lpa = m_LunarLabel->palette();
        QColor lunarTextColor = QColor("#000000");
        lunarTextColor.setAlphaF(0.5);
        lpa.setColor(QPalette::WindowText, lunarTextColor);
        m_LunarLabel->setPalette(lpa);
        m_LunarLabel->setForegroundRole(QPalette::WindowText);

        QPalette spa = m_SolarDay->palette();
        spa.setColor(QPalette::WindowText, Qt::red);
        m_SolarDay->setPalette(spa);
        m_SolarDay->setForegroundRole(QPalette::WindowText);
    } else if (type == 2) {
        m_leftground->setBColor("#282828");

        QPalette ypa = m_YearLabel->palette();
        QColor yearTextColor = QColor("#FFFFFF");
        yearTextColor.setAlphaF(0.8);
        ypa.setColor(QPalette::WindowText, yearTextColor);
        m_YearLabel->setPalette(ypa);
        m_YearLabel->setForegroundRole(QPalette::WindowText);

        QPalette lpa = m_LunarLabel->palette();
        QColor lunarTextColor = QColor("#FFFFFF");
        lunarTextColor.setAlphaF(0.5);
        lpa.setColor(QPalette::WindowText, lunarTextColor);
        m_LunarLabel->setPalette(lpa);
        m_LunarLabel->setForegroundRole(QPalette::WindowText);

        QPalette spa = m_SolarDay->palette();
        spa.setColor(QPalette::WindowText, Qt::red);
        m_SolarDay->setPalette(spa);
        m_SolarDay->setForegroundRole(QPalette::WindowText);
    }
    m_daymonthView->setTheMe(type);
    m_scheduleView->setTheMe(type);
}

/**
 * @brief CDayWindow::setYearData   设置选择时间年信息显示
 */
void CDayWindow::setYearData()
{
    QLocale locale;
    if (m_lunarVisible) {
        // 参考实现写作 tr("Y")/tr("M")/tr("D")，靠 zh_CN 翻译文件变成「年月日」。
        // 本项目没有接翻译（无 lupdate/lrelease、main() 里没装 QTranslator），
        // 直接按中文习惯拼接，与 calendarviews.cpp 的 "yyyy年 MMM" 保持一致。
        m_YearLabel->setText(QString("%1年%2月%3日")
                                 .arg(m_selectDate.year())
                                 .arg(m_selectDate.month())
                                 .arg(m_selectDate.day()));
    } else {
        m_YearLabel->setText(locale.toString(m_selectDate, "yyyy/M/d"));
    }
}

void CDayWindow::updateShowDate()
{
    setYearData();

    const int w = m_scheduleView->width() - 72;
    m_scheduleView->setRange(w, 1032, m_selectDate, m_selectDate);
    // 侧栏迷你月历需要 42 格：包含本月 1 号的那一周的周首日开始
    const QDate firstDay(m_selectDate.year(), m_selectDate.month(), 1);
    const int offset = (firstDay.dayOfWeek() - m_firstWeekday + 7) % 7;
    const QDate start = firstDay.addDays(-offset);
    QVector<QDate> monthDate;
    monthDate.reserve(DDEMonthCalendar::ItemSizeOfMonthDay);
    for (int i = 0; i < DDEMonthCalendar::ItemSizeOfMonthDay; ++i) {
        monthDate.append(start.addDays(i));
    }
    m_daymonthView->setShowDate(monthDate, m_selectDate, m_currentDateTime.date());

    if (m_lunarVisible) {
        updateShowLunar();
    }
}

/**
 * @brief CDayWindow::updateShowLunar       更新显示农历信息
 */
void CDayWindow::updateShowLunar()
{
    DDE25::LunarCache *cache = DDE25::LunarCache::instance();
    cache->ensureMonth(m_selectDate.year(), m_selectDate.month());

    const CaLunarDayInfo info = cache->info(m_selectDate);
    if (info.mLunarMonthName.isEmpty() && info.mLunarDayName.isEmpty()) {
        m_LunarLabel->clear();
        m_daymonthView->setLunarInfo(CaLunarDayInfo());
        return;
    }
    // 同上：参考实现是 tr("Lunar") + 月名 + 日名，这里直接用中文前缀
    m_LunarLabel->setText(QString("农历") + info.mLunarMonthName + info.mLunarDayName);
    m_daymonthView->setLunarInfo(info);
}

void CDayWindow::setLunarVisible(bool state)
{
    m_lunarVisible = state;
    m_LunarLabel->setVisible(state);
    m_SolarDay->setVisible(state);
    m_daymonthView->setLunarVisible(state);
    updateShowDate();
}

void CDayWindow::setCurrentDate(const QDate &date)
{
    if (!date.isValid() || date == m_selectDate) {
        return;
    }
    m_selectDate = date;
    updateShowDate();
    emit signalsCurrentDateChanged(m_selectDate);
}

void CDayWindow::setCurrentDateTime(const QDateTime &currentDate)
{
    m_currentDateTime = currentDate;
    m_scheduleView->setCurrentDate(currentDate);
}

void CDayWindow::setFirstWeekday(Qt::DayOfWeek weekday)
{
    m_firstWeekday = weekday;
    m_daymonthView->setFirstWeekDay(weekday);
    updateShowDate();
}

void CDayWindow::slotChangeSelectDate(const QDate &date)
{
    setCurrentDate(date);
}

void CDayWindow::slotSwitchPrePage()
{
    slotChangeSelectDate(m_selectDate.addDays(-1));
}

void CDayWindow::slotSwitchNextPage()
{
    slotChangeSelectDate(m_selectDate.addDays(1));
}

void CDayWindow::initUI()
{
    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(10, 9, 0, 3);

    m_YearLabel = new QLabel(this);
    m_YearLabel->setMinimumHeight(DDEDayCalendar::D_YLabelHeight);
    QFont labelF;
    labelF.setWeight(QFont::Medium);
    labelF.setPixelSize(DDECalendar::FontSizeTwentyfour);
    m_YearLabel->setFont(labelF);
    QPalette ypa = m_YearLabel->palette();
    QColor yearTextColor = QColor("#000000");
    yearTextColor.setAlphaF(0.8);
    ypa.setColor(QPalette::WindowText, yearTextColor);
    m_YearLabel->setPalette(ypa);
    titleLayout->addWidget(m_YearLabel);

    m_LunarLabel = new QLabel(this);
    titleLayout->addSpacing(15);
    m_LunarLabel->setFixedHeight(DDEDayCalendar::D_YLabelHeight);
    labelF.setPixelSize(DDECalendar::FontSizeFourteen);
    m_LunarLabel->setFont(labelF);
    m_LunarLabel->setAlignment(Qt::AlignCenter);
    QPalette lpa = m_LunarLabel->palette();
    QColor lunarTextColor = QColor("#000000");
    lunarTextColor.setAlphaF(0.5);
    lpa.setColor(QPalette::WindowText, lunarTextColor);
    m_LunarLabel->setPalette(lpa);
    titleLayout->addWidget(m_LunarLabel);

    m_SolarDay = new QLabel(this);
    labelF.setPixelSize(DDECalendar::FontSizeTen);
    m_SolarDay->setFixedHeight(DDEDayCalendar::D_YLabelHeight);
    m_SolarDay->setFont(labelF);
    m_SolarDay->setAlignment(Qt::AlignCenter);
    QPalette spa = m_SolarDay->palette();
    spa.setColor(QPalette::WindowText, Qt::red);
    m_SolarDay->setPalette(spa);
    titleLayout->addWidget(m_SolarDay);
    titleLayout->addStretch();

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);
    m_scheduleView = new CScheduleBodyView(this, CWeekGraphicsView::DayPos);
    m_scheduleView->setViewMargin(72, 109, 20, 0);
    m_scheduleView->setCurrentDate(QDateTime::currentDateTime());
    leftLayout->addLayout(titleLayout);
    leftLayout->addWidget(m_scheduleView);

    // 参考实现的 DVerticalLine：这里用 2px 的竖线代替
    m_verline = new QFrame(this);
    m_verline->setFrameShape(QFrame::VLine);
    m_verline->setFrameShadow(QFrame::Plain);
    m_verline->setFixedWidth(2);

    m_daymonthView = new CDayMonthView(this);
    m_daymonthView->setFirstWeekDay(m_firstWeekday);

    QHBoxLayout *leftMainLayout = new QHBoxLayout;
    leftMainLayout->setContentsMargins(0, 0, 0, 0);
    leftMainLayout->setSpacing(1);
    leftMainLayout->addLayout(leftLayout);
    leftMainLayout->addWidget(m_verline);
    leftMainLayout->addWidget(m_daymonthView);

    leftMainLayout->setStretchFactor(leftLayout, 3);
    leftMainLayout->setStretchFactor(m_verline, 1);
    leftMainLayout->setStretchFactor(m_daymonthView, 2);

    m_leftground = new CustomFrame(this);
    m_leftground->setRoundState(true, true, true, true);
    m_leftground->setLayout(leftMainLayout);
    m_leftground->setBColor("#FFFFFF");

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_leftground);

    this->setLayout(mainLayout);
}

void CDayWindow::initConnection()
{
    connect(m_daymonthView, &CDayMonthView::signalChangeSelectDate,
            this, &CDayWindow::slotChangeSelectDate);
    connect(m_scheduleView, &CScheduleBodyView::signalAngleDelta, this, [this](int delta) {
        if (delta > 0) {
            slotSwitchPrePage();
        } else if (delta < 0) {
            slotSwitchNextPage();
        }
    });
}

void CDayWindow::resizeEvent(QResizeEvent *event)
{
    // 日程区宽度变化后重新按 24 小时铺满时间网格
    const int w = m_scheduleView->width() - 72;
    m_scheduleView->setRange(w, 1032, m_selectDate, m_selectDate);
    QWidget::resizeEvent(event);
}

void CDayWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
        slotSwitchPrePage();
        break;
    case Qt::Key_Right:
        slotSwitchNextPage();
        break;
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}
