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

static const int MinYearValue = 1900;

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
    slideMonth(false);
}

void CalendarWindow::nextMonth()
{
    slideMonth(true);
}

void CalendarWindow::wheelEvent(QWheelEvent * e)
{
    if (e->angleDelta().y() < 0) {
        nextMonth();
    } else {
        if (m_infoView->year() > MinYearValue || m_infoView->month() != 1) {
            previousMonth();
        }
    }
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
    m_sidebarCalendar->setFixedSize(SidebarWidth, CalendarHeight);
    m_sidebarCalendar->setFirstWeekday(weekday);
    m_sidebarCalendar->setDate(QDate::currentDate());

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

    m_viewStack = new QStackedWidget;

    m_yearView = new YearView;
    m_yearView->setFixedSize(CalendarWidth, CalendarHeight);
    m_yearView->setFirstWeekday(weekday);
    m_yearView->setCurrentDate(QDate::currentDate());

    m_weekView = new WeekView;
    m_weekView->setFixedSize(CalendarWidth, CalendarHeight);
    m_weekView->setFirstWeekday(weekday);
    m_weekView->setCurrentDate(QDate::currentDate());

    m_dayView = new DayView;
    m_dayView->setFixedSize(CalendarWidth, CalendarHeight);
    m_dayView->setCurrentDate(QDate::currentDate());

    m_viewStack->addWidget(m_yearView);      // YearViewIndex
    m_viewStack->addWidget(m_calendarView);  // MonthViewIndex
    m_viewStack->addWidget(m_weekView);      // WeekViewIndex
    m_viewStack->addWidget(m_dayView);       // DayViewIndex

    m_dde25Page = new QWidget;
    m_dde25Page->setObjectName("Dde25Page");
    m_dde25Page->setStyleSheet("QWidget#Dde25Page { background: transparent; }");
    QHBoxLayout *dde25Layout = new QHBoxLayout(m_dde25Page);
    dde25Layout->setContentsMargins(0, 0, 0, 0);
    dde25Layout->setSpacing(0);
    dde25Layout->addWidget(m_sidebarCalendar);
    dde25Layout->addWidget(m_viewStack, 0, Qt::AlignHCenter);

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
    connect(m_weekView, &WeekView::dateClicked, this, [this](const QDate &date) {
        m_calendarView->setCurrentDate(date);
    });

    connect(m_calendarView, &CalendarView::currentDateChanged, [this](int year, int month){
        qDebug() << "current date changed" << year << month;
        m_infoView->blockSignals(true);
        m_infoView->setYear(year);
        m_infoView->setMonth(month);
        m_infoView->blockSignals(false);
        m_sidebarCalendar->setDate(m_calendarView->currentDate());
        m_yearView->setCurrentDate(m_calendarView->currentDate());
        m_weekView->setCurrentDate(m_calendarView->currentDate());
        m_dayView->setCurrentDate(m_calendarView->currentDate());
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

    connect(m_sidebarCalendar, &SidebarCalendarWidget::dateClicked, this, [this](const QDate &date) {
        m_calendarView->setCurrentDate(date);
    });
    connect(m_sidebarCalendar, &SidebarCalendarWidget::monthChanged, this, [this](int year, int month) {
        handleCurrentYearMonthChanged(year, month);
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
    });
    timer->start();
}

void CalendarWindow::setupMenu()
{
    DTitlebar *titlebar = this->titlebar();

    if (titlebar) {
        titlebar->setWindowFlags(titlebar->windowFlags() & ~Qt::WindowMaximizeButtonHint);
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
        m_layoutAction = titlebar->menu()->addAction(tr("Switch to DDE 25 layout"));

        titlebar->setCustomWidget(m_viewSwitcher, false);

        connect(titlebar->menu(), &QMenu::triggered, this, &CalendarWindow::menuItemInvoked);
    }
}


void CalendarWindow::menuItemInvoked(QAction *action)
{
    if (action == m_layoutAction) {
        const bool dde25 = m_settings->value("layout", QStringLiteral("dde25")).toString() != QStringLiteral("dde15");
        const bool newDde25 = !dde25;
        m_settings->setValue("layout", newDde25 ? QStringLiteral("dde25") : QStringLiteral("dde15"));
        applyLayout();
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
    m_settings->setValue("weekday", weekday);
}

void CalendarWindow::applyLayout() {
    const bool dde25 = m_settings->value("layout", QStringLiteral("dde25")).toString() != QStringLiteral("dde15");

    relayoutCalendarView(dde25);
    m_mainStack->setCurrentIndex(dde25 ? 1 : 0);

    m_viewSwitcher->setVisible(dde25);

    DTitlebar *titlebar = this->titlebar();
    const int titlebarHeight = titlebar ? titlebar->height() : 0;
    const int width = dde25 ? SidebarWidth + CalendarWidth
        : CalendarWidth + ContentLeftRightPadding * 2;
    const int contentHeight = dde25 ? CalendarHeight
        : InfoViewHeight + CalendarHeight;
    setFixedSize(width, contentHeight + titlebarHeight);

    updateLayoutActionText(dde25);
}

void CalendarWindow::relayoutCalendarView(bool dde25) {
    if (dde25) {
        if (m_viewStack->indexOf(m_calendarView) < 0) {
            m_viewStack->insertWidget(MonthViewIndex, m_calendarView);
        }
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
    m_calendarView->setLunarVisible(enable_Lunar ? true : QLocale::system().name().contains("zh"));
}

void CalendarWindow::slideMonth(bool next)
{
    m_animationContainer->show();
    m_animationContainer->raise();

    QPixmap one = getCalendarSnapshot();
    m_infoView->increaseMonth(next);
    QPixmap two = getCalendarSnapshot();
    QPixmap target = next ? joint(one, two) : joint(two, one);
    m_fakeContent->setPixmap(target);

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
