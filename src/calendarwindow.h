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
#include <QPushButton>
#include <QSettings>

#include <memory>

#include "calendarview.h"
#include "dde25/dde25common.h"
#include "schedule/dschedule.h"

DWIDGET_USE_NAMESPACE

class InfoView;
class SidebarCalendarWidget;
class SidebarScheduleList;
class CMonthWindow;
class CWeekWindow;
class CDayWindow;
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

    // 新建日程：M/W/D 的右键、双击空白处与标题栏「新建日程」按钮都走这里
    void slotCreateSchedule(const QDateTime &dateTime);
    // 编辑日程：双击日程块 / 日程块右键菜单「编辑」
    void slotEditSchedule(const DSchedule::Ptr &schedule);
    // 删除日程：日程块右键菜单「删除」。确认弹窗与重复日程的三种删法都在这里
    // （对应参考实现 CScheduleOperation::deleteSchedule）
    void slotDeleteSchedule(const DSchedule::Ptr &schedule);
    // 订阅在线日历的管理弹窗（标题栏菜单，在「设置窗口背景」下方）
    void slotManageIcsSubscription();

protected:
    void showEvent(QShowEvent *event) override;
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
    // 侧栏上半部分：小日历上高亮那天的日程列表（GXDE 日历原创，DDE 25 布局独有）
    SidebarScheduleList *m_sidebarScheduleList = nullptr;
    // 侧栏整列的容器：日程列表 + 迷你月历
    QWidget *m_sidebarContainer = nullptr;
    QFrame *m_sidebarSeparator = nullptr;

    // DDE 25 styled
    QStackedWidget *m_mainStack = nullptr;
    QWidget *m_dde25Page = nullptr;
    QStackedWidget *m_viewStack = nullptr;
    YearView *m_yearView = nullptr;
    WeekView *m_weekView = nullptr;
    DayView *m_dayView = nullptr;
    // 移植自 dde-calendar 的月/周/日视图，仅在 DDE 25 布局下使用
    CMonthWindow *m_monthWindow = nullptr;
    CWeekWindow *m_weekWindow = nullptr;
    CDayWindow *m_dayWindow = nullptr;
    ViewSwitcher *m_viewSwitcher = nullptr;
    QPushButton *m_newScheduleButton = nullptr;
    QPushButton *m_sidebarToggleButton = nullptr;
    bool m_sidebarCollapsed = false;
    QVBoxLayout *m_dde15Layout = nullptr;

    QFrame * m_animationContainer = nullptr;
    QLabel * m_fakeContent = nullptr;
    QLabel * m_icon;

    QPropertyAnimation * m_scrollAnimation = nullptr;

    // DDE 15 翻月的滚轮节流（见 DDE25::WheelStepper）
    std::unique_ptr<DDE25::WheelStepper> m_wheelStepper;

    void initUI();
    void initAnimation();
    void initDateChangeMonitor();
    void setupMenu();
    // 把「管理在线日历」挪到 DTK 内置的「设置窗口背景」下面，见实现里的时序说明
    void repositionIcsAction();
    // 翻月动画：正数为往后翻 |count| 个月，负数为往前翻
    void slideMonth(int count);
    QPixmap getCalendarSnapshot() const;
    QPixmap joint(QPixmap & top, QPixmap & bottom) const;
    void updateTime() const;
    void updateSentense() const;
    void updateDate() const;
    void updateDde25CurrentTime();

    // Style changer
    void applyLayout();
    void relayoutCalendarView(bool dde25);
    void updateLayoutActionText(bool dde25);
    void setWeekday(int weekday);
    void setSidebarCollapsed(bool collapsed);

    QAction *m_monAction;
    QAction *m_tueAction;
    QAction *m_wedAction;
    QAction *m_thuAction;
    QAction *m_friAction;
    QAction *m_satAction;
    QAction *m_sunAction;
    QAction *m_layoutAction;
    // 「管理在线日历」：DTK 的内置菜单项在 showEvent 里才追加，位置见 setupMenu()
    QAction *m_icsAction = nullptr;
    bool m_icsActionRepositioned = false;

    QSettings *m_settings;
    QSettings *m_dateSettings;

    QStringList m_sentenseData = {"", ""};
};

#endif // CALENDARWINDOW_H
