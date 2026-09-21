/*
 * Copyright (C) 2026 CharOfString <root@charofstring.cc>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ----------------------------------------------------------------------------
 * 本文件为 GXDE 日历原创。
 */

#ifndef SIDEBARSCHEDULELIST_H
#define SIDEBARSCHEDULELIST_H

#include <QColor>
#include <QDate>
#include <QVector>
#include <QWidget>

#include "schedule/dschedule.h"

class QLabel;
class QPushButton;
class QScrollArea;
class QVBoxLayout;

/**
 * @brief 侧栏日程列表里的一行：左侧类型色条 + 标题 + 时间。
 *
 * 参考实现的侧栏只有日历（账户/类型）树，没有当天日程列表；这里是 GXDE 日历
 * 自己加的，绘制风格跟月视图的日程块保持一致（左侧 4px 色条 + 类型色）。
 */
class SidebarScheduleItem : public QWidget
{
    Q_OBJECT
public:
    explicit SidebarScheduleItem(QWidget *parent = nullptr);

    void setData(const DSchedule::Ptr &schedule);
    DSchedule::Ptr getData() const { return m_schedule; }

signals:
    // 单击一行 -> 打开编辑弹窗
    void signalClicked(const DSchedule::Ptr &schedule);

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QString timeText() const;

    DSchedule::Ptr m_schedule;
    QColor m_color;
    bool m_hover = false;
};

/**
 * @brief 侧栏上半部分的「选中那天的日程」列表。
 *
 * 小日历上高亮哪天，这里就列哪天的日程：日期由外部（CalendarWindow）在
 * CalendarView::currentDateChanged 里同步过来，日程增删改则在
 * CalendarService::scheduleUpdate 里重查一遍。
 * 迷你日历中的 addStretch() 把它顶到侧栏底部，上方这块就是本控件。
 */
class SidebarScheduleList : public QWidget
{
    Q_OBJECT
public:
    explicit SidebarScheduleList(QWidget *parent = nullptr);

    // 设置要显示的日期，日期没变时不做任何事（重查请用 refresh()）
    void setDate(const QDate &date);
    QDate date() const { return m_date; }
    // 重新查一遍当前日期的日程（日程增删改后调用）
    void refresh();

signals:
    void signalEditSchedule(const DSchedule::Ptr &schedule);
    void signalCreateSchedule(const QDate &date);

protected:
    void showEvent(QShowEvent *event) override;

private:
    void rebuild();
    void updateHeader();

    QDate m_date;
    QLabel *m_dateLabel = nullptr;
    QPushButton *m_addButton = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QWidget *m_itemContainer = nullptr;
    QVBoxLayout *m_itemLayout = nullptr;
    QLabel *m_emptyLabel = nullptr;
    QVector<SidebarScheduleItem *> m_items;
};

#endif // SIDEBARSCHEDULELIST_H
