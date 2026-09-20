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

#ifndef CALENDARWINDOW_H
#define CALENDARWINDOW_H

#include <DMainWindow>
#include <QLabel>
#include <QSettings>

#include "calendarview.h"

DWIDGET_USE_NAMESPACE

class InfoView;
class SidebarCalendarWidget;
class YearView;
class WeekView;
class DayView;
class ViewSwitcher;
class QPropertyAnimation;
class QStackedWidget;
class QVBoxLayout;

class CalendarWindow : public DMainWindow
{
    Q_OBJECT
public:
    CalendarWindow();

public slots:
    void handleTodayButtonClicked();
    void handleCurrentYearMonthChanged(int year, int month);

    void previousMonth();
    void nextMonth();

protected:
    void wheelEvent(QWheelEvent *);

protected slots:
    void menuItemInvoked(QAction *action);


private:
    void initLunar();

private:
    InfoView * m_infoView = nullptr;
    CalendarView * m_calendarView = nullptr;
    QFrame * m_contentBackground = nullptr;
    SidebarCalendarWidget * m_sidebarCalendar = nullptr;

    // DDE 25 styled
    QStackedWidget *m_mainStack = nullptr;
    QWidget *m_dde25Page = nullptr;
    QStackedWidget *m_viewStack = nullptr;
    YearView *m_yearView = nullptr;
    WeekView *m_weekView = nullptr;
    DayView *m_dayView = nullptr;
    ViewSwitcher *m_viewSwitcher = nullptr;
    QVBoxLayout *m_dde15Layout = nullptr;

    QFrame * m_animationContainer = nullptr;
    QLabel * m_fakeContent = nullptr;
    QLabel * m_icon;

    QPropertyAnimation * m_scrollAnimation = nullptr;

    void initUI();
    void initAnimation();
    void initDateChangeMonitor();
    void setupMenu();
    void slideMonth(bool next);
    QPixmap getCalendarSnapshot() const;
    QPixmap joint(QPixmap & top, QPixmap & bottom) const;
    void updateTime() const;
    void updateSentense() const;
    void updateDate() const;

    // Style changer
    void applyLayout();
    void relayoutCalendarView(bool dde25);
    void updateLayoutActionText(bool dde25);
    void setWeekday(int weekday);

    QAction *m_monAction;
    QAction *m_tueAction;
    QAction *m_wedAction;
    QAction *m_thuAction;
    QAction *m_friAction;
    QAction *m_satAction;
    QAction *m_sunAction;
    QAction *m_layoutAction;

    QSettings *m_settings;
    QSettings *m_dateSettings;

    QStringList m_sentenseData = {"", ""};
};

#endif // CALENDARWINDOW_H
