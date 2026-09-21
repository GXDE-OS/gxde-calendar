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
 * 差异：Stage 1 不含日程数据，日程区以只画时间网格的 CScheduleBodyView 代替
 *       （见 schedulebodyview.*）；不含搜索、提醒浮框与触摸手势。
 */

#ifndef WEEKWINDOW_H
#define WEEKWINDOW_H

#include <QDate>
#include <QVector>
#include <QWidget>

class CScheduleBodyView;
class CWeekHeadView;
class CWeekView;
class CustomFrame;
class QLabel;
class QPushButton;

/**
 * @brief The CWeekWindow class
 * 周视图窗口：66px 头部（年份 + 农历年 + 周数条 + Week + Today）+ 日期表头 + 日程区
 */
class CWeekWindow : public QWidget
{
    Q_OBJECT
public:
    explicit CWeekWindow(QWidget *parent = nullptr);
    ~CWeekWindow() override;

    // 设置当前选中日期
    void setCurrentDate(const QDate &date);
    QDate currentDate() const { return m_currentDate; }
    // 设置一周的起始日
    void setFirstWeekday(Qt::DayOfWeek weekday);
    void setTheMe(int type = 0);
    void setLunarVisible(bool visible);

signals:
    // 选中某一天（双击表头，用于切到日视图）
    void signalsSelectDate(QDate date);
    // 当前选中日期发生变化
    void signalsCurrentDateChanged(QDate date);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void slotPrevWeek();
    void slotNextWeek();
    void slotToday();
    void slotWeekNumSelectDate(const QDate &date);

private:
    void setYearData();
    void updateLunarYearLabel();
    void updateShowDate();
    // 以 date 为选中日，刷新所有子控件
    void switchDate(const QDate &date);

    CWeekHeadView *m_weekHeadView = nullptr;
    CScheduleBodyView *m_weekBody = nullptr;
    CWeekView *m_weekview = nullptr;
    CustomFrame *m_todayframe = nullptr;
    QLabel *m_yearLabel = nullptr;
    QLabel *m_yearLunarLabel = nullptr;
    QLabel *m_weekLabel = nullptr;
    QPushButton *m_today = nullptr;

    QDate m_currentDate;
    QDate m_startDate;
    QDate m_endDate;
    Qt::DayOfWeek m_firstWeekday = Qt::Monday;
    int m_themetype = 0;

    QVector<QDate> m_days;
};

#endif // WEEKWINDOW_H
