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
 * 移植自 dde-calendar（src/calendar-client/src/widget/weekWidget/weekwindow.*）。
 */

#include "weekwindow.h"

#include "constants.h"
#include "customframe.h"
#include "dde25common.h"
#include "schedule/calendarservice.h"
#include "schedulebodyview.h"
#include "schedulequery.h"
#include "weekgraphicsview.h"
#include "weekheadview.h"
#include "weeknumview.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>
#include <QVBoxLayout>

// --------------------------------------------------------------- CWeekWindow

CWeekWindow::CWeekWindow(QWidget *parent)
    : QWidget(parent)
{
    setContentsMargins(0, 0, 0, 0);

    m_currentDate = QDate::currentDate();

    // ---------------- 头部 ----------------
    // 与 monthwindow / daymonthview 一致：文案在构造时定好，不随后续日期变化
    m_today = new QPushButton(tr("Today"), this);
    m_today->setObjectName("WeekWindowTodayButton");
    m_today->setAccessibleName("WeekWindowTodayButton");
    m_today->setFixedSize(DDEWeekCalendar::WTodayWindth, DDEWeekCalendar::WTodayHeight);
    QFont todayfont;
    todayfont.setWeight(QFont::Medium);
    todayfont.setPixelSize(DDECalendar::FontSizeFourteen);
    m_today->setFont(todayfont);

    m_yearLabel = new QLabel(this);
    m_yearLabel->setFixedHeight(DDEWeekCalendar::W_YLabelHeight);
    QFont t_labelF;
    t_labelF.setWeight(QFont::Medium);
    t_labelF.setPixelSize(DDECalendar::FontSizeTwentyfour);
    m_yearLabel->setFont(t_labelF);

    m_yearLunarLabel = new QLabel(this);
    m_yearLunarLabel->setFixedSize(DDEWeekCalendar::W_YLunatLabelWindth,
                                   DDEWeekCalendar::W_YLunatLabelHeight);
    QFont yLabelF;
    yLabelF.setWeight(QFont::Medium);
    yLabelF.setPixelSize(DDECalendar::FontSizeFourteen);
    m_yearLunarLabel->setFont(yLabelF);
    m_yearLunarLabel->setAlignment(Qt::AlignCenter);

    m_weekview = new CWeekView(this);

    m_weekLabel = new QLabel(this);
    m_weekLabel->setFixedHeight(DDEWeekCalendar::W_YLabelHeight);
    QFont weeklabelF;
    weeklabelF.setWeight(QFont::Medium);
    weeklabelF.setPixelSize(DDECalendar::FontSizeFourteen);
    m_weekLabel->setFont(weeklabelF);
    m_weekLabel->setText(tr("Week"));

    m_todayframe = new CustomFrame(this);
    m_todayframe->setContentsMargins(0, 0, 0, 0);
    m_todayframe->setRoundState(true, true, true, true);
    m_todayframe->setFixedHeight(DDEWeekCalendar::W_YLabelHeight);
    m_todayframe->setboreder(1);
    QHBoxLayout *todaylayout = new QHBoxLayout;
    todaylayout->setContentsMargins(0, 0, 0, 0);
    todaylayout->setSpacing(0);
    todaylayout->addWidget(m_weekview);
    m_todayframe->setLayout(todaylayout);

    QHBoxLayout *yeartitleLayout = new QHBoxLayout;
    yeartitleLayout->setContentsMargins(0, 0, 0, 0);
    yeartitleLayout->setSpacing(0);
    yeartitleLayout->addSpacing(10);
    yeartitleLayout->addWidget(m_yearLabel);
    yeartitleLayout->addSpacing(6);
    yeartitleLayout->addWidget(m_yearLunarLabel);
    yeartitleLayout->addStretch();
    yeartitleLayout->addWidget(m_todayframe, 0, Qt::AlignCenter);
    yeartitleLayout->addSpacing(10);
    yeartitleLayout->addWidget(m_weekLabel, 0, Qt::AlignCenter);
    yeartitleLayout->addStretch();
    yeartitleLayout->addWidget(m_today, 0, Qt::AlignRight);

    QWidget *top = new QWidget(this);
    top->setFixedHeight(DDEMonthCalendar::M_YTopHeight);
    top->setLayout(yeartitleLayout);

    // ---------------- 日期表头 + 日程区 ----------------
    m_weekHeadView = new CWeekHeadView(this);
    m_weekBody = new CScheduleBodyView(this, CWeekGraphicsView::WeekPos);
    // 左边距与表头月份标签宽度一致，使日程区各天分隔线与表头纵向对齐
    m_weekBody->setViewMargin(DDEWeekCalendar::WMCellHeaderWidth - 5, 109 + 30, 0, 0);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_weekHeadView, 1);
    mainLayout->addWidget(m_weekBody, 9);

    QVBoxLayout *hhLayout = new QVBoxLayout;
    hhLayout->setContentsMargins(0, 0, 0, 0);
    hhLayout->setSpacing(0);
    hhLayout->addWidget(top);
    hhLayout->addLayout(mainLayout);

    QHBoxLayout *tMainLayout = new QHBoxLayout;
    tMainLayout->setContentsMargins(0, 0, 0, 0);
    tMainLayout->setSpacing(0);
    tMainLayout->addLayout(hhLayout);
    setLayout(tMainLayout);

    // ---------------- 信号 ----------------
    connect(m_today, &QPushButton::clicked, this, &CWeekWindow::slotToday);
    connect(m_weekview, &CWeekView::signalBtnPrev, this, &CWeekWindow::slotPrevWeek);
    connect(m_weekview, &CWeekView::signalBtnNext, this, &CWeekWindow::slotNextWeek);
    connect(m_weekview, &CWeekView::signalsSelectDate, this, &CWeekWindow::slotWeekNumSelectDate);
    connect(m_weekHeadView, &CWeekHeadView::signalsViewSelectDate, this, [this](const QDate &date) {
        // 双击表头某天 = 选中该天并请求切到日视图
        switchDate(date);
        emit signalsSelectDate(date);
    });
    // 滚轮：delta > 0 为上一周，< 0 为下一周。连续滚动按累计周数一次跳到位
    m_wheelStepper = std::make_unique<DDE25::WheelStepper>(
        [this](int steps) {
            setCurrentDate(m_currentDate.addDays(-steps * DDEWeekCalendar::AFewDaysofWeek));
        },
        DDE25::kWheelCooldownMs);
    const auto angleDeltaToSlide = [this](int delta) { m_wheelStepper->step(delta); };
    connect(m_weekview, &CWeekView::signalAngleDelta, this, angleDeltaToSlide);
    connect(m_weekHeadView, &CWeekHeadView::signalAngleDelta, this, angleDeltaToSlide);
    connect(m_weekBody, &CScheduleBodyView::signalAngleDelta, this, angleDeltaToSlide);
    // 新建/编辑日程的入口往上抛给 CalendarWindow，弹窗由它统一负责
    connect(m_weekBody, &CScheduleBodyView::signalCreateSchedule,
            this, &CWeekWindow::signalCreateSchedule);
    connect(m_weekBody, &CScheduleBodyView::signalEditSchedule,
            this, &CWeekWindow::signalEditSchedule);
    connect(m_weekBody, &CScheduleBodyView::signalDeleteSchedule,
            this, &CWeekWindow::signalDeleteSchedule);

    // 日程增删改都发 scheduleUpdate()，重查一遍就能刷新日程块
    connect(CalendarService::instance(), &CalendarService::scheduleUpdate,
            this, &CWeekWindow::updateShowDate);

    setTheMe(DDE25::themeType());
    setCurrentDate(QDate::currentDate());
}

CWeekWindow::~CWeekWindow() = default;

void CWeekWindow::setCurrentDate(const QDate &date)
{
    if (!date.isValid()) {
        return;
    }

    // 日期没变就不重建，理由同 CMonthWindow::setCurrentDate
    if (date == m_currentDate) {
        return;
    }

    m_currentDate = date;
    // 隐藏时只记日期，显示出来再重建，理由同 CMonthWindow::setCurrentDate
    if (isVisible()) {
        refreshCurrentDate();
    } else {
        m_dateRefreshPending = true;
    }

    emit signalsCurrentDateChanged(m_currentDate);
}

/**
 * @brief CWeekWindow::refreshCurrentDate  按 m_currentDate 重建各子控件
 */
void CWeekWindow::refreshCurrentDate()
{
    m_dateRefreshPending = false;
    updateShowDate();
    m_weekview->setCurrent(QDateTime::currentDateTime());
    m_weekview->setSelectDate(m_currentDate);
    setYearData();
    updateLunarYearLabel();
}

void CWeekWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if (m_dateRefreshPending) {
        refreshCurrentDate();
    }
}

void CWeekWindow::setCurrentDateTime(const QDateTime &currentDate) {
    const bool dateRolledOver = m_currentDateTime.date() != currentDate.date();
    m_currentDateTime = currentDate;
    m_weekBody->setCurrentDate(currentDate);

    if (dateRolledOver) {
        updateShowDate();
    }
}

void CWeekWindow::setFirstWeekday(Qt::DayOfWeek weekday)
{
    m_firstWeekday = weekday;
    m_weekview->setFirstWeekDay(weekday);
    m_weekview->setSelectDate(m_currentDate);
    updateShowDate();
}

void CWeekWindow::setTheMe(int type)
{
    m_themetype = type;

    QColor yearTextColor = type == 2 ? QColor("#FFFFFF") : QColor("#000000");
    yearTextColor.setAlphaF(0.8);
    QColor lunarTextColor = type == 2 ? QColor("#FFFFFF") : QColor("#000000");
    lunarTextColor.setAlphaF(0.5);

    QPalette pa = m_yearLabel->palette();
    pa.setColor(QPalette::WindowText, yearTextColor);
    m_yearLabel->setPalette(pa);

    QPalette lunaPa = m_yearLunarLabel->palette();
    lunaPa.setColor(QPalette::WindowText, lunarTextColor);
    m_yearLunarLabel->setPalette(lunaPa);

    QPalette wpa = m_weekLabel->palette();
    wpa.setColor(QPalette::WindowText, QColor("#717171"));
    m_weekLabel->setPalette(wpa);

    if (type == 2) {
        QColor bColor = "#FFFFFF";
        bColor.setAlphaF(0.05);
        m_todayframe->setBColor(bColor);
        m_todayframe->setBorderColor(QColor(0, 0, 0, 0));
    } else {
        m_todayframe->setBColor(Qt::white);
        m_todayframe->setBorderColor(QColor(0, 0, 0, 0x1e));
    }

    m_weekview->setTheMe(type);
    m_weekHeadView->setTheMe(type);
    m_weekBody->setTheMe(type);
    update();
}

void CWeekWindow::setLunarVisible(bool visible)
{
    m_weekHeadView->setLunarVisible(visible);
    m_yearLunarLabel->setVisible(visible);
}

void CWeekWindow::setYearData()
{
    // 按钮文案固定为 Today，不做「非今日时显示 Return Today」的切换
    m_yearLabel->setText(QString::number(m_currentDate.year()));
}

void CWeekWindow::updateLunarYearLabel()
{
    DDE25::LunarCache::instance()->ensureMonth(m_currentDate.year(), m_currentDate.month());
    const CaLunarDayInfo info = DDE25::LunarCache::instance()->info(m_currentDate);
    if (info.mGanZhiYear.isEmpty()) {
        m_yearLunarLabel->clear();
        return;
    }

    QString zodiac = info.mZodiac;
    // 生肖在黄历数据里形如 "鼠"，直接拼接比去查表更稳
    m_yearLunarLabel->setText(QString("-%1%2年-").arg(info.mGanZhiYear, zodiac));
}

void CWeekWindow::updateShowDate()
{
    m_days = DDE25::weekDates(m_currentDate, m_firstWeekday);
    if (m_days.isEmpty()) {
        return;
    }
    m_startDate = m_days.first();
    m_endDate = m_days.last();

    m_weekHeadView->setWeekDay(m_days, m_currentDate);

    // 确保表头 7 天与选中日所在月的农历数据都已缓存
    DDE25::LunarCache *cache = DDE25::LunarCache::instance();
    for (const QDate &date : m_days) {
        cache->ensureMonth(date.year(), date.month());
    }
    m_weekHeadView->updateLunar();

    m_weekBody->setRange(m_startDate, m_endDate);

    // 查询只在这里做，不进 paintEvent
    m_weekBody->setScheduleInfo(DDE25::querySchedules(m_startDate, m_endDate));
}

void CWeekWindow::switchDate(const QDate &date)
{
    setCurrentDate(date);
}

void CWeekWindow::slotPrevWeek()
{
    switchDate(m_currentDate.addDays(-DDEWeekCalendar::AFewDaysofWeek));
}

void CWeekWindow::slotNextWeek()
{
    switchDate(m_currentDate.addDays(DDEWeekCalendar::AFewDaysofWeek));
}

void CWeekWindow::slotToday()
{
    switchDate(QDate::currentDate());
}

void CWeekWindow::slotWeekNumSelectDate(const QDate &date)
{
    // 点击周数条只切换显示周，不改变选中日所在日
    setCurrentDate(date);
}

void CWeekWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    // 键盘连发也走节流：+1 = 滚轮向上 = 上一周
    case Qt::Key_Left:
        m_wheelStepper->nudge(1);
        break;
    case Qt::Key_Right:
        m_wheelStepper->nudge(-1);
        break;
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

void CWeekWindow::resizeEvent(QResizeEvent *event)
{
    // 与 dde-calendar 一致：周数条的宽度随窗口宽度变化（含 1 个按钮的宽度）
    qreal dw = width() * 0.4186 + 0.5;
    m_weekview->setFixedSize(qRound(dw + 36), DDEWeekCalendar::W_YLabelHeight);

    QWidget::resizeEvent(event);
}
