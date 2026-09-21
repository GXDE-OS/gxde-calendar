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
 * 差异：去掉「时间跳转对话框」与搜索图标按钮；月份滚轮在 monthdayview.* 中实现。
 */

#ifndef MONTHWINDOW_H
#define MONTHWINDOW_H

#include <QDate>
#include <QMap>
#include <QVector>
#include <QWidget>

class CMonthDayView;
class CMonthView;
class CustomFrame;
class QLabel;
class QPushButton;

/**
 * @brief 月视图窗口：66px 头部（年份 + 农历年 + 月份滚轮 + Today）+ 月视图
 */
class CMonthWindow : public QWidget
{
    Q_OBJECT
public:
    explicit CMonthWindow(QWidget *parent = nullptr);

    void setCurrentDate(const QDate &date);
    void setFirstWeekday(Qt::DayOfWeek weekday);
    void setTheMe(int type = 0);
    void setLunarVisible(bool visible);
    void setFestival(const QMap<QDate, int> &festivalInfo);

signals:
    // 选中某个日期（切到日视图等）
    void signalsSelectDate(QDate date);
    // 当前显示的月份发生变化
    void signalsCurrentDateChanged(QDate date);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void setYearData();
    void updateLunarYearLabel();
    // 切换月份：next 为 true 时下一个月
    void slideMonth(bool next);

    CMonthView *m_monthView = nullptr;
    CMonthDayView *m_monthDayView = nullptr;
    QLabel *m_yearLabel = nullptr;
    QLabel *m_yearLunarLabel = nullptr;
    QPushButton *m_today = nullptr;

    QDate m_currentDate;
    int m_themetype = 0;
};

#endif // MONTHWINDOW_H
