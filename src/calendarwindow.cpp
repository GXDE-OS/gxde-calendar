/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     kirigaya <kirigaya@mkacg.com>
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

#include "calendarwindow.h"
#include "dbuscalendar_adaptor.h"
#include "constants.h"
#include "infoview.h"
#include "sidebarcalendarwidget.h"
#include "calendarviews.h"
#include "viewswitcher.h"
#include "dde25/dde25common.h"
#include "dde25/monthwindow.h"
#include "dde25/daywindow.h"
#include "dde25/weekwindow.h"
#include "dde25/scheduledlg.h"
#include "dde25/schedulectrldlg.h"
#include "dde25/sidebarschedulelist.h"
#include "icssubscriptiondlg.h"
#include "schedule/calendarservice.h"

#include <QDate>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QStackedWidget>
#include <QPropertyAnimation>
#include <QWheelEvent>
#include <QPainter>
#include <QMenu>
#include <DTitlebar>
#include <DAboutDialog>
#include <DHiDPIHelper>

DWIDGET_USE_NAMESPACE

static const int CalendarHeaderHeight = 60;

static const int CalendarWidth = 760;
static const int CalendarHeight = 500;

static const int InfoViewWidth = CalendarWidth - 86;
static const int InfoViewHeight = 90;

static const int ContentLeftRightPadding = 80;

static const int SidebarWidth = 220;
static const int ViewSwitcherWidth = 160;
static const int ViewSwitcherHeight = 24;
static const int Dde25SidePadding = 16;

static const int MinYearValue = 1900;
static const int Dde15FlipAnimationMs = 300;

enum ViewIndex {
    YearViewIndex = 0,
    MonthViewIndex = 1,
    WeekViewIndex = 2,
    DayViewIndex = 3,
};

CalendarWindow::CalendarWindow() :
    DMainWindow(nullptr)
{
    setContentsMargins(QMargins(0, 0, 0, 0));

    m_settings = new QSettings;
    m_dateSettings = new QSettings("deepin", "dde-dock-datetime", this);

    initUI();
    initAnimation();
    m_wheelStepper = std::make_unique<DDE25::WheelStepper>(
        // 原实现：滚轮向下（delta < 0）翻下一个月，向上翻上一个月。
        // step 传来的档数带 delta 的符号，slideMonth 正数才是下个月，所以这里取负
        [this](int steps) { slideMonth(-steps); }, Dde15FlipAnimationMs);
    initDateChangeMonitor();
    initLunar();

    setWindowTitle(tr("GXDE Calendar"));

    new CalendarAdaptor(this);

    setEnableWindowBackground(1);
}

void CalendarWindow::handleTodayButtonClicked()
{
    m_calendarView->setCurrentDate(QDate::currentDate());
}

void CalendarWindow::handleCurrentYearMonthChanged(int year, int month)
{
    QDate changedDate;
    changedDate.setDate(year, month,  1);

    const uint daysInMonth = changedDate.daysInMonth();
    const uint currentDay = QDate::currentDate().day();
    if (currentDay > daysInMonth) {
        changedDate = changedDate.addDays(daysInMonth - 1);
    } else {
        changedDate = changedDate.addDays(currentDay - 1);
    }

    m_calendarView->setCurrentDate(changedDate);
}

void CalendarWindow::previousMonth()
{
    slideMonth(-1);
}

void CalendarWindow::nextMonth()
{
    slideMonth(1);
}

void CalendarWindow::wheelEvent(QWheelEvent * e)
{
    // 滚轮向下（delta < 0）往后翻。连续滚动累计后一次跳到位，见 DDE25::WheelStepper
    m_wheelStepper->step(e->angleDelta().y());
}

void CalendarWindow::initUI()
{
    const int weekday = m_settings->value("weekday", Sunday).toInt();

    // ---------------- DDE 15 styled ----------------
    m_contentBackground = new QFrame;
    m_contentBackground->setObjectName("CalendarBackground");
    m_contentBackground->setStyleSheet("QFrame#CalendarBackground { "
                             "background:#00ffffff;"
                             "}");
    m_contentBackground->setFixedSize(CalendarWidth + ContentLeftRightPadding * 2,
                                      InfoViewHeight + CalendarHeight);

    m_icon = new QLabel(this);
    m_icon->setFixedSize(24, 24);
    // 修复高分屏下图标模糊的问题
    // m_icon->setPixmap(DHiDPIHelper::loadNxPixmap(":/resources/icon/gxde-calendar.svg")
    //                  .scaled(m_icon->size() * devicePixelRatioF()));
    m_icon->setPixmap(QIcon::fromTheme("gxde-calendar").pixmap(m_icon->size()));
    m_icon->move(12, 8);

    m_infoView = new InfoView;
    m_infoView->setStyleSheet("QFrame { background: rgba(0, 0, 0, 0) }");
    m_infoView->setFixedSize(InfoViewWidth, InfoViewHeight);
    m_infoView->setYearRange(MinYearValue, INT_MAX);

    m_infoView->setYear(QDate::currentDate().year());
    m_infoView->setMonth(QDate::currentDate().month());

    m_calendarView = new CalendarView;
    m_calendarView->setFixedSize(CalendarWidth, CalendarHeight);
    m_calendarView->setFirstWeekday(weekday);
    m_calendarView->setCurrentDate(QDate::currentDate());

    m_sidebarCalendar = new SidebarCalendarWidget;
    m_sidebarCalendar->setFixedWidth(SidebarWidth);
    m_sidebarCalendar->setFirstWeekday(weekday);
    m_sidebarCalendar->setDate(QDate::currentDate());
    m_sidebarCalendar->setObjectName("SidebarCalendarWidget");
    m_sidebarCalendar->setAttribute(Qt::WA_StyledBackground, true);
    m_sidebarCalendar->setStyleSheet(
        "QWidget#SidebarCalendarWidget {"
        "  background-color: #67f9f9fa;"
        "  border: none;"
        "}");

    m_animationContainer = new QFrame(m_contentBackground);
    m_animationContainer->setStyleSheet("QFrame { background: rgba(0, 0, 0, 0) }");
    m_animationContainer->setFixedSize(m_calendarView->width(),
                                       m_calendarView->height() - CalendarHeaderHeight);
    m_animationContainer->move(ContentLeftRightPadding, CalendarHeaderHeight + InfoViewHeight);
    m_animationContainer->hide();

    m_fakeContent = new QLabel(m_animationContainer);
    m_fakeContent->setStyleSheet("QLabel { background: rgba(0, 0, 0, 0) }");
    m_fakeContent->setFixedSize(m_animationContainer->width(),
                                m_animationContainer->height() * 2);

    m_dde15Layout = new QVBoxLayout;
    m_dde15Layout->setContentsMargins(0, 0, 0, 0);
    m_dde15Layout->setSpacing(0);
    m_dde15Layout->addWidget(m_infoView, 0, Qt::AlignHCenter);
    // Month view m_calendarView is imported dynamically by applyLayout()
    m_contentBackground->setLayout(m_dde15Layout);

    // ---------------- DDE 25 styled ----------------
    m_viewSwitcher = new ViewSwitcher;
    m_viewSwitcher->setLabels(QStringList() << tr("Y") << tr("M") << tr("W")
        << tr("D"));
    m_viewSwitcher->setFixedSize(ViewSwitcherWidth, ViewSwitcherHeight);

    // 侧边栏折叠按钮，样式对齐 gxde-file-manager 标题栏按钮
    // （白底 + 1px rgba 边框 + 4px 圆角 + 蓝色渐变 hover）
    m_sidebarToggleButton = new QPushButton;
    m_sidebarToggleButton->setObjectName("SidebarToggleButton");
    // 图标用 QIcon::Normal/Active 双态：常态深色、hover 时自动切白色，
    // 尺寸由 setIconSize 精确控制为 16x16（高 DPI 下由 SVG 引擎清晰渲染）。
    QIcon sidebarIcon;
    sidebarIcon.addFile(":/resources/icon/sidebar.svg", QSize(16, 16), QIcon::Normal);
    sidebarIcon.addFile(":/resources/icon/sidebar_dark.svg", QSize(16, 16), QIcon::Active);
    m_sidebarToggleButton->setIcon(sidebarIcon);
    m_sidebarToggleButton->setIconSize(QSize(16, 16));
    m_sidebarToggleButton->setFixedSize(24, 24);
    m_sidebarToggleButton->setFocusPolicy(Qt::NoFocus);
    m_sidebarToggleButton->setCursor(Qt::PointingHandCursor);
    m_sidebarToggleButton->setStyleSheet(
        "QPushButton#SidebarToggleButton {"
        "  background-color: white;"
        "  border: 1px solid rgba(0, 0, 0, 0.1);"
        "  border-radius: 4px;"
        "}"
        "QPushButton#SidebarToggleButton:hover {"
        "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #8CCFFF, stop:1 #4BB8FF);"
        "  border: 1px solid #3caafd;"
        "}"
        "QPushButton#SidebarToggleButton:pressed {"
        "  background-color: #2ca7f8;"
        "  border: 1px solid #1088ff;"
        "}");
    connect(m_sidebarToggleButton, &QPushButton::clicked, this, [this](bool) {
        setSidebarCollapsed(!m_sidebarCollapsed);
    });
    m_sidebarCollapsed = m_settings->value("sidebarCollapsed", false).toBool();

    // 新建日程按钮，与侧边栏折叠按钮同一套样式，放在 Y/M/W/D 后面
    m_newScheduleButton = new QPushButton;
    m_newScheduleButton->setObjectName("NewScheduleButton");
    // 图标取自 dde-calendar 的 dde_calendar_create 内置图标（本机没有图标主题，
    // 已内置进 qrc）；深浅主题各一份，选法同 cpushbutton.cpp
    m_newScheduleButton->setIcon(QIcon(DDE25::themeType() == 2
                                           ? ":/resources/icon/dde_calendar_create_dark.svg"
                                           : ":/resources/icon/dde_calendar_create_light.svg"));
    m_newScheduleButton->setIconSize(QSize(16, 16));
    m_newScheduleButton->setFixedSize(24, 24);
    m_newScheduleButton->setToolTip(tr("New Schedule"));
    m_newScheduleButton->setFocusPolicy(Qt::NoFocus);
    m_newScheduleButton->setCursor(Qt::PointingHandCursor);
    m_newScheduleButton->setStyleSheet(
        "QPushButton#NewScheduleButton {"
        "  background-color: white;"
        "  border: 1px solid rgba(0, 0, 0, 0.1);"
        "  border-radius: 4px;"
        "}"
        "QPushButton#NewScheduleButton:hover {"
        "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #8CCFFF, stop:1 #4BB8FF);"
        "  border: 1px solid #3caafd;"
        "}"
        "QPushButton#NewScheduleButton:pressed {"
        "  background-color: #2ca7f8;"
        "  border: 1px solid #1088ff;"
        "}");
    // 没有点具体位置时，就以当前选中日期 + 当前时刻（弹窗里会向上取整到
    // 15 分钟）作为新建日程的默认时间
    connect(m_newScheduleButton, &QPushButton::clicked, this, [this](bool) {
        slotCreateSchedule(QDateTime(m_calendarView->currentDate(), QTime::currentTime()));
    });

    m_viewStack = new QStackedWidget;

    const auto makeStretchable = [](QWidget *w) {
        w->setMinimumSize(CalendarWidth, CalendarHeight);
        w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    };

    m_yearView = new YearView;
    makeStretchable(m_yearView);
    m_yearView->setFirstWeekday(weekday);
    m_yearView->setCurrentDate(QDate::currentDate());

    m_weekView = new WeekView;
    m_weekView->setFixedSize(CalendarWidth, CalendarHeight);
    m_weekView->setFirstWeekday(weekday);
    m_weekView->setCurrentDate(QDate::currentDate());

    m_dayView = new DayView;
    m_dayView->setFixedSize(CalendarWidth, CalendarHeight);
    m_dayView->setCurrentDate(QDate::currentDate());

    // DDE 25 的月视图换成移植自 dde-calendar 的 CMonthWindow；
    // m_calendarView 只在 DDE 15 布局里显示，由 relayoutCalendarView() 动态挂载。
    m_monthWindow = new CMonthWindow;
    makeStretchable(m_monthWindow);
    m_monthWindow->setFirstWeekday(DDE25::fromGxdeWeekday(weekday));
    m_monthWindow->setCurrentDate(QDate::currentDate());
    m_monthWindow->setTheMe(DDE25::themeType());

    // 周视图同样换成移植自 dde-calendar 的 CWeekWindow
    m_weekWindow = new CWeekWindow;
    makeStretchable(m_weekWindow);
    m_weekWindow->setFirstWeekday(DDE25::fromGxdeWeekday(weekday));
    m_weekWindow->setCurrentDate(QDate::currentDate());
    m_weekWindow->setTheMe(DDE25::themeType());

    // 日视图同样换成移植自 dde-calendar 的 CDayWindow
    m_dayWindow = new CDayWindow;
    makeStretchable(m_dayWindow);
    m_dayWindow->setFirstWeekday(DDE25::fromGxdeWeekday(weekday));
    m_dayWindow->setCurrentDate(QDate::currentDate());
    m_dayWindow->setTheMe(DDE25::themeType());

    m_viewStack->addWidget(m_yearView);       // YearViewIndex
    m_viewStack->addWidget(m_monthWindow);    // MonthViewIndex
    m_viewStack->addWidget(m_weekWindow);     // WeekViewIndex
    m_viewStack->addWidget(m_dayWindow);      // DayViewIndex

    m_dde25Page = new QWidget;
    m_dde25Page->setObjectName("Dde25Page");
    m_dde25Page->setStyleSheet("QWidget#Dde25Page { background: transparent; }");

    // 手写的 WeekView/DayView 已被 CWeekWindow/CDayWindow 取代。保留对象
    // （成员仍被引用），但挂到 DDE 25 页面上并隐藏，避免成为游离的顶层窗口。
    m_weekView->setParent(m_dde25Page);
    m_weekView->hide();
    m_dayView->setParent(m_dde25Page);
    m_dayView->hide();

    // 侧栏整列：上半部分是「迷你月历上高亮那天」的日程列表，下半部分是迷你月历
    // 本身（迷你月历自己的布局把内容顶到底部）。参考实现的侧栏是日历（账户/类型）
    // 树 + 迷你月历，GXDE 日历这边把上半部分换成了当天日程列表。
    m_sidebarScheduleList = new SidebarScheduleList;
    m_sidebarScheduleList->setDate(QDate::currentDate());

    m_sidebarContainer = new QWidget(m_dde25Page);
    m_sidebarContainer->setObjectName("SidebarContainer");
    m_sidebarContainer->setAttribute(Qt::WA_StyledBackground, true);
    m_sidebarContainer->setStyleSheet(
        "QWidget#SidebarContainer {"
        "  background-color: #67f9f9fa;"
        "  border: none;"
        "}");
    m_sidebarContainer->setFixedWidth(SidebarWidth);

    QVBoxLayout *sidebarLayout = new QVBoxLayout(m_sidebarContainer);
    sidebarLayout->setContentsMargins(0, 8, 0, 0);
    sidebarLayout->setSpacing(0);
    sidebarLayout->addWidget(m_sidebarScheduleList, 1);
    sidebarLayout->addWidget(m_sidebarCalendar);

    QHBoxLayout *dde25Layout = new QHBoxLayout(m_dde25Page);
    dde25Layout->setContentsMargins(0, 0, Dde25SidePadding, 0);
    dde25Layout->setSpacing(0);
    dde25Layout->addWidget(m_sidebarContainer);

    m_sidebarSeparator = new QFrame(m_dde25Page);
    m_sidebarSeparator->setObjectName("SidebarSeparator");
    m_sidebarSeparator->setFixedWidth(1);
    m_sidebarSeparator->setStyleSheet(DDE25::separatorStyleSheet());
    dde25Layout->addWidget(m_sidebarSeparator);

    dde25Layout->addSpacing(Dde25SidePadding);
    dde25Layout->addWidget(m_viewStack, 1);

    // ---------------- Main stack ----------------
    m_mainStack = new QStackedWidget;
    m_mainStack->setObjectName("RootBackground");
    m_mainStack->setStyleSheet("QStackedWidget#RootBackground { background: transparent; }");
    m_mainStack->addWidget(m_contentBackground);
    m_mainStack->addWidget(m_dde25Page);

    setCentralWidget(m_mainStack);

    connect(m_viewSwitcher, &ViewSwitcher::currentChanged,
        m_viewStack, &QStackedWidget::setCurrentIndex);
    m_viewSwitcher->setCurrentIndex(MonthViewIndex);

    connect(m_yearView, &YearView::dateClicked, this, [this](const QDate &date) {
        m_calendarView->setCurrentDate(date);
        m_viewSwitcher->setCurrentIndex(MonthViewIndex);
    });
    // 双击周视图表头某天 = 选中该天并切到日视图（对齐 dde-calendar 的 signalSwitchView(3)）
    connect(m_weekWindow, &CWeekWindow::signalsSelectDate, this, [this](const QDate &date) {
        m_calendarView->setCurrentDate(date);
        m_viewSwitcher->setCurrentIndex(DayViewIndex);
    });

    connect(m_calendarView, &CalendarView::currentDateChanged, [this](int year, int month){
        qDebug() << "current date changed" << year << month;
        m_infoView->blockSignals(true);
        m_infoView->setYear(year);
        m_infoView->setMonth(month);
        m_infoView->blockSignals(false);
        m_sidebarCalendar->setDate(m_calendarView->currentDate());
        // 侧栏日程列表只认小日历上高亮的那天，跟着唯一日期源一起走
        m_sidebarScheduleList->setDate(m_calendarView->currentDate());
        m_yearView->setCurrentDate(m_calendarView->currentDate());
        m_weekWindow->setCurrentDate(m_calendarView->currentDate());
        m_dayWindow->setCurrentDate(m_calendarView->currentDate());
        m_monthWindow->setCurrentDate(m_calendarView->currentDate());
    });
    // DDE 25 月视图选中日期：沿用 m_calendarView 作为唯一的日期来源，
    // 再由上面的 currentDateChanged 广播回各视图（含 DDE 15 的 InfoView）。
    connect(m_monthWindow, &CMonthWindow::signalsSelectDate, this, [this](const QDate &date) {
        m_calendarView->setCurrentDate(date);
    });
    // 月视图的月份条、周视图的周数条都只是「换个时间看」，统一回灌到 m_calendarView。
    // CalendarView::setCurrentDate 对相同日期直接返回，不会形成信号回环。
    connect(m_monthWindow, &CMonthWindow::signalsCurrentDateChanged, this, [this](const QDate &date) {
        m_calendarView->setCurrentDate(date);
    });
    // 新建/编辑/删除日程：三个视图的入口最后都汇到这里，弹窗开在窗口上（不受视图切页影响）
    connect(m_monthWindow, &CMonthWindow::signalCreateSchedule,
            this, &CalendarWindow::slotCreateSchedule);
    connect(m_monthWindow, &CMonthWindow::signalEditSchedule,
            this, &CalendarWindow::slotEditSchedule);
    connect(m_monthWindow, &CMonthWindow::signalDeleteSchedule,
            this, &CalendarWindow::slotDeleteSchedule);
    connect(m_weekWindow, &CWeekWindow::signalCreateSchedule,
            this, &CalendarWindow::slotCreateSchedule);
    connect(m_weekWindow, &CWeekWindow::signalEditSchedule,
            this, &CalendarWindow::slotEditSchedule);
    connect(m_weekWindow, &CWeekWindow::signalDeleteSchedule,
            this, &CalendarWindow::slotDeleteSchedule);
    connect(m_dayWindow, &CDayWindow::signalCreateSchedule,
            this, &CalendarWindow::slotCreateSchedule);
    connect(m_dayWindow, &CDayWindow::signalEditSchedule,
            this, &CalendarWindow::slotEditSchedule);
    connect(m_dayWindow, &CDayWindow::signalDeleteSchedule,
            this, &CalendarWindow::slotDeleteSchedule);
    connect(m_weekWindow, &CWeekWindow::signalsCurrentDateChanged, this, [this](const QDate &date) {
        m_calendarView->setCurrentDate(date);
    });
    connect(m_dayWindow, &CDayWindow::signalsCurrentDateChanged, this, [this](const QDate &date) {
        m_calendarView->setCurrentDate(date);
    });
    connect(m_calendarView, &CalendarView::currentFestivalChanged, m_infoView, &InfoView::setFestival);
    connect(m_calendarView, &CalendarView::refreshSentenseFinished, m_calendarView, [this](QStringList data){
        m_sentenseData = data;
        updateSentense();
    });
    connect(m_infoView, &InfoView::todayButtonClicked,
            this, &CalendarWindow::handleTodayButtonClicked);

    connect(m_infoView, &InfoView::yearChanged, [this](int year) {
        const int month = m_infoView->month();
        handleCurrentYearMonthChanged(year, month);
    });
    connect(m_infoView, &InfoView::monthChanged, [this](int month) {
        const int year = m_infoView->year();
        handleCurrentYearMonthChanged(year, month);
    });

    // 侧栏小日历只当「侧栏日程列表的日期选择器」用：点它只换侧栏列表，不动主视图。
    // 以前是把日期灌进 m_calendarView，于是右边 M/W/D 会跟着跳到那一天；
    // 反向（主视图动 → 小日历跟着走）仍然保留，见上面的 currentDateChanged 广播
    connect(m_sidebarCalendar, &SidebarCalendarWidget::dateClicked, this, [this](const QDate &date) {
        m_sidebarScheduleList->setDate(date);
    });
    // 小日历上的 ‹ › 只翻它自己显示的月份（控件内部自己处理），同样不动主视图，
    // 所以 monthChanged 不接：主视图的月份仍然由 m_calendarView 那一路驱动

    // 侧栏日程列表：日程增删改都发 scheduleUpdate()，重查一遍即可
    connect(CalendarService::instance(), &CalendarService::scheduleUpdate,
            m_sidebarScheduleList, &SidebarScheduleList::refresh);
    connect(m_sidebarScheduleList, &SidebarScheduleList::signalEditSchedule,
            this, &CalendarWindow::slotEditSchedule);
    // 列表上的「+」按当前日期新建，时刻取当前时间（弹窗里会向上取整到 15 分钟）
    connect(m_sidebarScheduleList, &SidebarScheduleList::signalCreateSchedule, this,
            [this](const QDate &date) {
                slotCreateSchedule(QDateTime(date, QTime::currentTime()));
            });

    setupMenu();
    applyLayout();
}

void CalendarWindow::initAnimation()
{
    m_scrollAnimation = new QPropertyAnimation(m_fakeContent, "pos");
    m_scrollAnimation->setDuration(300);

    connect(m_scrollAnimation, &QPropertyAnimation::finished, [this]{
        m_animationContainer->hide();
    });
}

void CalendarWindow::initDateChangeMonitor()
{
    static QDate LastCurrentDate = QDate::currentDate();
    updateDate();
    updateTime();

    QTimer * timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, [this] {
        QDate currentDate = QDate::currentDate();
        if (LastCurrentDate != currentDate) {
            LastCurrentDate = currentDate;
            updateDate();
        }
        updateTime();
        updateDde25CurrentTime();
    });
    timer->start();
}

void CalendarWindow::updateDde25CurrentTime() {
    const QDateTime now = QDateTime::currentDateTime();
    if (m_dayWindow && m_dayWindow->isVisible()) {
        m_dayWindow->setCurrentDateTime(now);
    } else if (m_weekWindow && m_weekWindow->isVisible()) {
        m_weekWindow->setCurrentDateTime(now);
    } else if (m_monthWindow && m_monthWindow->isVisible()) {
        m_monthWindow->setCurrentDateTime(now);
    }
}

void CalendarWindow::setupMenu()
{
    DTitlebar *titlebar = this->titlebar();

    if (titlebar) {
        titlebar->setMenu(new QMenu(titlebar));
        titlebar->setSeparatorVisible(true);

        QMenu *firstWeekday = titlebar->menu()->addMenu(tr("First Day of Week"));
        QLocale locale;

        m_monAction = firstWeekday->addAction(locale.dayName(1, QLocale::ShortFormat));
        m_tueAction = firstWeekday->addAction(locale.dayName(2, QLocale::ShortFormat));
        m_wedAction = firstWeekday->addAction(locale.dayName(3, QLocale::ShortFormat));
        m_thuAction = firstWeekday->addAction(locale.dayName(4, QLocale::ShortFormat));
        m_friAction = firstWeekday->addAction(locale.dayName(5, QLocale::ShortFormat));
        m_satAction = firstWeekday->addAction(locale.dayName(6, QLocale::ShortFormat));
        m_sunAction = firstWeekday->addAction(locale.dayName(7, QLocale::ShortFormat));

        titlebar->menu()->addSeparator();
        m_layoutAction = titlebar->menu()->addAction(tr("Switch to DDE 15 layout"));
        m_icsAction = titlebar->menu()->addAction(tr("Manage Online Calendars"));

        // 标题栏自定义区域：左侧只有侧边栏折叠按钮，Y/M/W/D 紧随其后
        // 左边距 = DTitlebar 主布局的 6px + 这里的 12px，与 gxde-file-manager
        // 标题栏 logo 的位置一致（dfilemanagerwindow.cpp:970-973）
        QWidget *titleCenter = new QWidget;
        QHBoxLayout *titleCenterLayout = new QHBoxLayout(titleCenter);
        titleCenterLayout->setContentsMargins(0, 0, 0, 0);
        titleCenterLayout->setSpacing(0);
        titleCenterLayout->addSpacing(12);
        titleCenterLayout->addWidget(m_sidebarToggleButton, 0, Qt::AlignVCenter);
        titleCenterLayout->addSpacing(12);
        titleCenterLayout->addWidget(m_viewSwitcher, 0, Qt::AlignVCenter);
        titleCenterLayout->addSpacing(8);
        titleCenterLayout->addWidget(m_newScheduleButton, 0, Qt::AlignVCenter);
        titleCenterLayout->addStretch();
        titlebar->setCustomWidget(titleCenter, false);

        connect(titlebar->menu(), &QMenu::triggered, this, &CalendarWindow::menuItemInvoked);
    }
}


void CalendarWindow::menuItemInvoked(QAction *action)
{
    if (action == m_layoutAction) {
        m_dde25Layout = !m_dde25Layout;
        applyLayout();
        return;
    }

    if (action == m_icsAction) {
        slotManageIcsSubscription();
        return;
    }

    if (action == m_monAction) {
        setWeekday(Monday);
        return;
    }

    if (action == m_tueAction) {
        setWeekday(Tuesday);
        return;
    }

    if (action == m_wedAction) {
        setWeekday(Wednesday);
        return;
    }

    if (action == m_thuAction) {
        setWeekday(Thursday);
        return;
    }

    if (action == m_friAction) {
        setWeekday(Friday);
        return;
    }

    if (action == m_satAction) {
        setWeekday(Saturday);
        return;
    }

    if (action == m_sunAction) {
        setWeekday(Sunday);
        return;
    }
}

void CalendarWindow::setWeekday(int weekday) {
    m_calendarView->setFirstWeekday(weekday);
    m_sidebarCalendar->setFirstWeekday(weekday);
    m_monthWindow->setFirstWeekday(DDE25::fromGxdeWeekday(weekday));
    m_weekWindow->setFirstWeekday(DDE25::fromGxdeWeekday(weekday));
    m_dayWindow->setFirstWeekday(DDE25::fromGxdeWeekday(weekday));
    m_settings->setValue("weekday", weekday);

}

void CalendarWindow::setSidebarCollapsed(bool collapsed) {
    if (m_sidebarCollapsed == collapsed) {
        return;
    }

    m_sidebarCollapsed = collapsed;
    m_settings->setValue("sidebarCollapsed", collapsed);
    applyLayout();
}

void CalendarWindow::applyLayout() {
    const bool dde25 = m_dde25Layout;

    relayoutCalendarView(dde25);
    m_mainStack->setCurrentIndex(dde25 ? 1 : 0);

    m_viewSwitcher->setVisible(dde25);
    m_newScheduleButton->setVisible(dde25);
    m_sidebarToggleButton->setVisible(dde25);
    m_sidebarContainer->setVisible(dde25 && !m_sidebarCollapsed);
    m_sidebarCalendar->setVisible(dde25 && !m_sidebarCollapsed);
    m_sidebarSeparator->setVisible(dde25 && !m_sidebarCollapsed);

    DTitlebar *titlebar = this->titlebar();
    const int titlebarHeight = titlebar ? titlebar->height() : 0;

    if (titlebar) {
        // DDE 25 允许最大化；DDE 15 是固定尺寸的，最大化按钮没有意义
        Qt::WindowFlags flags = titlebar->windowFlags();
        if (dde25) {
            flags |= Qt::WindowMaximizeButtonHint;
        } else {
            flags &= ~Qt::WindowMaximizeButtonHint;
        }
        titlebar->setWindowFlags(flags);
    }

    if (dde25) {
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        setMinimumSize(SidebarWidth + CalendarWidth + Dde25SidePadding * 2,
            CalendarHeight + titlebarHeight);
    } else {
        if (isMaximized()) {
            showNormal();
        }
        const int width = CalendarWidth + ContentLeftRightPadding * 2;
        const int contentHeight = InfoViewHeight + CalendarHeight;
        setFixedSize(width, contentHeight + titlebarHeight);
    }

    updateLayoutActionText(dde25);
}

void CalendarWindow::relayoutCalendarView(bool dde25) {
    if (dde25) {
        // DDE 25 用移植来的 CMonthWindow，m_calendarView 在这里只是隐藏的日期源。
        // 它已经不参加布局，但仍挂在 DDE 25 页面上，避免成为游离的顶层窗口。
        if (m_viewStack->indexOf(m_calendarView) >= 0) {
            m_viewStack->removeWidget(m_calendarView);
        }
        if (m_calendarView->parentWidget() != m_dde25Page) {
            m_calendarView->setParent(m_dde25Page);
        }
        m_calendarView->hide();
        if (m_viewStack->indexOf(m_monthWindow) < 0) {
            m_viewStack->insertWidget(MonthViewIndex, m_monthWindow);
        }
        m_monthWindow->setCurrentDate(m_calendarView->currentDate());
        m_viewStack->setCurrentIndex(m_viewSwitcher->currentIndex());
    } else {
        if (m_dde15Layout->indexOf(m_calendarView) < 0) {
            m_viewStack->removeWidget(m_calendarView);
            m_dde15Layout->addWidget(m_calendarView, 0, Qt::AlignHCenter);
        }
        m_calendarView->show();
    }
}

void CalendarWindow::updateLayoutActionText(bool dde25)
{
    if (m_layoutAction) {
        m_layoutAction->setText(dde25 ? tr("Switch to DDE 15 layout")
            : tr("Switch to DDE 25 layout"));
    }
}

void CalendarWindow::initLunar()
{
    const bool enable_Lunar = m_settings->value("EnableLunar", false).toBool();
    const bool lunarVisible = enable_Lunar ? true : QLocale::system().name().contains("zh");
    m_calendarView->setLunarVisible(lunarVisible);
    m_monthWindow->setLunarVisible(lunarVisible);
    m_weekWindow->setLunarVisible(lunarVisible);
    m_dayWindow->setLunarVisible(lunarVisible);
}

/**
 * @brief CalendarWindow::slideMonth  翻月动画
 * @param count 正数为往后翻 |count| 个月，负数为往前翻
 *
 * 一次翻多个月也只 grab 两次、只起一次动画：滚轮连发时按累计月数跳一次，
 * 而不是每个档位都重新截图 + 起一次动画。
 */
void CalendarWindow::slideMonth(int count)
{
    if (count == 0) {
        return;
    }

    const QDate current(m_infoView->year(), m_infoView->month(), 1);
    QDate target = current.addMonths(count);
    if (target.year() < MinYearValue) {
        // 往前翻到头就停在最早的一年一月
        target = QDate(MinYearValue, 1, 1);
    }
    if (target == current) {
        return;
    }

    m_animationContainer->show();
    m_animationContainer->raise();

    QPixmap one = getCalendarSnapshot();

    // 直接把年月拨到目标月：上面那两个 signal 每个都会带回一次整屏刷新，
    // 这里挡掉中间的过渡月份，只让 handleCurrentYearMonthChanged 触发一次切换
    m_infoView->blockSignals(true);
    m_infoView->setYear(target.year());
    m_infoView->setMonth(target.month());
    m_infoView->blockSignals(false);
    handleCurrentYearMonthChanged(target.year(), target.month());

    QPixmap two = getCalendarSnapshot();
    const bool next = count > 0;
    QPixmap pixmap = next ? joint(one, two) : joint(two, one);
    m_fakeContent->setPixmap(pixmap);

    m_scrollAnimation->setStartValue(QPoint(0, next ? 0 : -one.height()));
    m_scrollAnimation->setEndValue(QPoint(0, next ? -one.height() : 0));

    m_scrollAnimation->start();
}

QPixmap CalendarWindow::getCalendarSnapshot() const
{
    return m_calendarView->grab(m_calendarView->rect().adjusted(0, CalendarHeaderHeight, 0, 0));
}

QPixmap CalendarWindow::joint(QPixmap &top, QPixmap &bottom) const
{
    QPixmap target(qMax(top.width(), bottom.width()),
                   top.height() + bottom.height());

    target.fill(Qt::white);
    QPainter painter;
    painter.begin(&target);
    painter.drawPixmap(0, 0, top);
    painter.drawPixmap(0, top.height(), bottom);
    painter.end();

    return target;
}

void CalendarWindow::updateSentense() const
{
    if (QLocale::system().name().contains("zh")) {
        QString senShow = m_sentenseData.at(0);
        if (m_sentenseData.at(1) != "") {
            senShow += "--" + m_sentenseData.at(1);
        }
        m_infoView->setSentense(senShow);
        return;
    }
    QString senShow = m_sentenseData.at(2);
    if (m_sentenseData.at(1) != "") {
        senShow += "--" + m_sentenseData.at(3);
    }
    m_infoView->setSentense(senShow);
}

void CalendarWindow::slotCreateSchedule(const QDateTime &dateTime)
{
    // type = 1：新建（参考实现 Calendarmainwindow::slotNewSchedule）
    CScheduleDlg scheduleDlg(1, this, false);
    scheduleDlg.setDate(dateTime);
    scheduleDlg.exec();
}

void CalendarWindow::slotEditSchedule(const DSchedule::Ptr &schedule)
{
    if (schedule.isNull()) {
        return;
    }

    // type = 0：编辑（参考实现 DragInfoGraphicsView::contextMenuEvent 的编辑分支）
    CScheduleDlg scheduleDlg(0, this, false);
    scheduleDlg.setData(schedule);
    scheduleDlg.exec();
}

namespace {

/**
 * @brief changeRepetitionRule   把重复日程的重复规则截到选中那一次之前
 *
 * 移植自参考实现 CScheduleOperation::changeRepetitionRule()。删除「本次及以后」
 * 时先用它改原日程的重复规则：按次数重复的改成剩余次数，永不结束 / 结束于日期的
 * 把结束日期挪到选中那天的前一天。改完是删是更新由调用方按规则还剩不剩日程决定。
 */
void changeRepetitionRule(DSchedule::Ptr &newinfo, const DSchedule::Ptr &oldinfo)
{
    const int num = DSchedule::numberOfRepetitions(newinfo, oldinfo->dtStart());
    if (newinfo->recurrence()->duration() > 0) {
        //按次数重复：只留选中这次之前的
        const int duration = num - 1;
        if (duration > 1) {
            newinfo->recurrence()->setDuration(duration);
        } else {
            //剩不下第二次就不算重复日程了
            newinfo->setRRuleType(DSchedule::RRule_None);
        }
    } else {
        //永不结束 / 结束于日期：结束日期挪到选中那天的前一天
        newinfo->recurrence()->setDuration(0);
        newinfo->recurrence()->setEndDateTime(oldinfo->dtStart().addDays(-1));
    }
}

} // namespace

void CalendarWindow::slotDeleteSchedule(const DSchedule::Ptr &schedule)
{
    if (schedule.isNull()) {
        return;
    }

    // 视图给的是展开后的那一次（重复日程的 dtStart 是这一次的开始时间），而改重复
    // 规则、删日程都要在原始日程上做，所以先按 UID 回库里取原始日程
    // （参考实现 CScheduleOperation::deleteSchedule 同样是先取原始数据）
    DSchedule::Ptr origin = CalendarService::instance()->getScheduleByScheduleID(schedule->uid());
    if (origin.isNull()) {
        return;
    }

    // 确认弹窗与按钮文案都取自参考实现 CScheduleOperation::deleteSchedule()：
    // 普通日程问一次，重复日程还要问删哪几次
    CScheduleCtrlDlg msgBox(this);
    msgBox.setText(tr("You are deleting an event."));

    if (origin->getRRuleType() == DSchedule::RRule_None) {
        msgBox.setInformativeText(tr("Are you sure you want to delete this event?"));
        msgBox.addPushButton(tr("Cancel", "button"), true);
        msgBox.addWaringButton(tr("Delete", "button"), true);
        msgBox.exec();
        if (msgBox.clickButton() == 1) {
            CalendarService::instance()->deleteScheduleByScheduleID(origin->uid());
        }
        return;
    }

    // 重复日程：numberOfRepetitions() 数的是「截止这次一共重复了几次」，
    // 第一次就是 1 —— 它前面没有可保留的，所以没有「本次及以后」这个选项
    if (DSchedule::numberOfRepetitions(origin, schedule->dtStart()) == 1) {
        msgBox.setInformativeText(
            tr("Do you want to delete all occurrences of this event, or only the "
               "selected occurrence?"));
        msgBox.addPushButton(tr("Cancel", "button"));
        msgBox.addPushButton(tr("Delete All"));
        msgBox.addWaringButton(tr("Delete Only This Event"));
        msgBox.exec();

        switch (msgBox.clickButton()) {
        case 1:
            CalendarService::instance()->deleteScheduleByScheduleID(origin->uid());
            break;
        case 2:
            //仅删这一次：把这次加进重复规则的忽略列表，日程本身留在库里
            origin->recurrence()->addExDateTime(schedule->dtStart());
            CalendarService::instance()->updateSchedule(origin);
            break;
        default:
            break;
        }
        return;
    }

    msgBox.setInformativeText(
        tr("Do you want to delete this and all future occurrences of this event, or "
           "only the selected occurrence?"));
    msgBox.addPushButton(tr("Cancel", "button"));
    msgBox.addPushButton(tr("Delete All Future Events"));
    msgBox.addWaringButton(tr("Delete Only This Event"));
    msgBox.exec();

    switch (msgBox.clickButton()) {
    case 1: {
        //删除本次及以后：截断重复规则后判断还剩不剩日程
        const QList<QDateTime> exDt = origin->recurrence()->exDateTimes();
        changeRepetitionRule(origin, schedule);
        if (origin->getRRuleType() == DSchedule::RRule_None
                && exDt.contains(origin->dtStart())) {
            //截断后不重复了，而它自己的开始时间又早在忽略列表里，等于一次都不剩
            CalendarService::instance()->deleteScheduleByScheduleID(origin->uid());
        } else {
            CalendarService::instance()->updateSchedule(origin);
        }
        break;
    }
    case 2:
        origin->recurrence()->addExDateTime(schedule->dtStart());
        CalendarService::instance()->updateSchedule(origin);
        break;
    default:
        break;
    }
}

void CalendarWindow::showEvent(QShowEvent *event)
{
    DMainWindow::showEvent(event);

    // DTitlebar::showEvent() 里才把「设置窗口背景」这些内置项追加到菜单末尾，
    // 而它跟本窗口的 showEvent 谁先谁后不保证。挪位置又不能在菜单弹出过程中做
    // （在 aboutToShow 里 insertAction() 会改到正在布局的 action 列表，xcb 下会崩），
    // 所以推到事件循环下一轮：那时菜单已经彻底建好，离用户点开也还早。
    if (m_icsAction != nullptr && !m_icsActionRepositioned) {
        QTimer::singleShot(0, this, [this] { repositionIcsAction(); });
    }
}

void CalendarWindow::repositionIcsAction() {
    m_icsActionRepositioned = true;

    DTitlebar *titlebar = this->titlebar();
    if (titlebar == nullptr || titlebar->menu() == nullptr || m_icsAction == nullptr) {
        return;
    }

    QMenu *menu = titlebar->menu();
    const QList<QAction *> actions = menu->actions();
    const QString backgroundText =
        QCoreApplication::translate("TitleBarMenu", "Set Window Background");

    for (int i = 0; i < actions.size(); i++) {
        if (actions.at(i)->text() != backgroundText) {
            continue;
        }

        QAction *next = (i + 1 < actions.size()) ? actions.at(i + 1) : nullptr;
        if (next == m_icsAction) {
            return;
        }
        if (next != nullptr) {
            menu->insertAction(next, m_icsAction);
        } else {
            menu->addAction(m_icsAction);
        }
        return;
    }
}

void CalendarWindow::slotManageIcsSubscription() {
    CIcsSubscriptionDlg dlg(this);
    dlg.exec();
}

void CalendarWindow::updateTime() const
{
    m_infoView->setTime(QDateTime::currentDateTime().toString(m_dateSettings->value("24HourFormat", true).toBool() ? "hh:mm" : "hh:mm A"));
}

void CalendarWindow::updateDate() const
{
    QDate currentDate = QDate::currentDate();
    m_calendarView->setCurrentDate(currentDate);

    DTitlebar *titlebar = this->titlebar();
    if (titlebar) {
        titlebar->setTitle(currentDate.toString(Qt::ISODate));
    }
}
