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
 * Origin copyright bearer: 2017 - 2026 UnionTech Software Technology Co., Ltd.
 * This file is ported from DDE calendar tag 6.6.0
 * Minimal modification is applied to make the class build against
 * DTK2Widget-Qt6.
 * ----------------------------------------------------------------------------
 * 移植自 dde-calendar（src/calendar-client/src/widget/dayWidget/daywindow.*）。
 * 差异：基类 CScheduleBaseWidget 改为 QWidget（本阶段无日程数据层），
 *       日程区用 CScheduleBodyView(DayPos) 代替 CScheduleView，
 *       不含搜索、提醒浮框与拖拽新建。
 */

#ifndef DAYWINDOW_H
#define DAYWINDOW_H

#include <QDate>
#include <QDateTime>
#include <QWidget>

#include <memory>

#include "dde25common.h"
#include "schedule/dschedule.h"

class CDayMonthView;
class CScheduleBodyView;
class CustomFrame;
class QFrame;
class QLabel;

/**
 * @brief The CDayWindow class
 * 日视图：66px 头部（日期 + 农历）+ 时间网格 + 右侧栏（迷你月历 + 黄历）
 */
class CDayWindow : public QWidget
{
    Q_OBJECT
public:
    explicit CDayWindow(QWidget *parent = nullptr);
    ~CDayWindow() override;

    // 设置当前选中日期
    void setCurrentDate(const QDate &date);
    QDate currentDate() const { return m_selectDate; }
    // 设置当前时间（用于「今天」判断与当前时刻线）
    void setCurrentDateTime(const QDateTime &currentDate);
    // 设置一周起始日
    void setFirstWeekday(Qt::DayOfWeek weekday);
    void setTheMe(int type = 0);
    void setLunarVisible(bool visible);

signals:
    // 当前选中日期发生变化
    void signalsCurrentDateChanged(QDate date);
    // 请求新建日程（日程区右键菜单 / 双击空白处）
    void signalCreateSchedule(QDateTime dateTime);
    // 请求编辑日程（双击日程块 / 右键菜单「编辑」）
    void signalEditSchedule(DSchedule::Ptr schedule);
    // 请求删除日程（右键菜单「删除」）
    void signalDeleteSchedule(DSchedule::Ptr schedule);

public slots:
    // 更新选择时间
    void slotChangeSelectDate(const QDate &date);

private slots:
    // 切换选择时间
    void slotSwitchPrePage();
    void slotSwitchNextPage();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void initUI();
    void initConnection();
    // 刷新头部、日程区与右侧栏
    void updateShowDate();
    // 刷新农历/黄历显示
    void updateShowLunar();
    // 设置头部年月日显示
    void setYearData();

    CDayMonthView *m_daymonthView = nullptr;
    CustomFrame *m_leftground = nullptr;
    QFrame *m_verline = nullptr;
    QLabel *m_YearLabel = nullptr;
    QLabel *m_LunarLabel = nullptr;
    QLabel *m_SolarDay = nullptr;
    CScheduleBodyView *m_scheduleView = nullptr;

    QDate m_selectDate;
    QDateTime m_currentDateTime = QDateTime::currentDateTime();
    Qt::DayOfWeek m_firstWeekday = Qt::Monday;
    bool m_lunarVisible = true;
    int m_themetype = 0;

    // 日程区滚轮的换天节流（见 DDE25::WheelStepper）
    std::unique_ptr<DDE25::WheelStepper> m_wheelStepper;

    // 隐藏期间攒下的刷新，显示出来时补做
    bool m_dateRefreshPending = false;
};

#endif // DAYWINDOW_H
