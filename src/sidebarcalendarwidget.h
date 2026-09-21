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
 * Original author: kirigaya <kirigaya@mkacg.com>
 * Adapted from the DDE calendar sidebar calendar widget, modified to adapt the
 * GXDE calendar.
 * ----------------------------------------------------------------------------
 * 改自 dde-calendar 的侧边栏小日历，为适配 GXDE 日历做过修改。
 */

#ifndef SIDEBARCALENDARWIDGET_H
#define SIDEBARCALENDARWIDGET_H

#include <QWidget>
#include <QDate>
#include <QPushButton>

#include <dimagebutton.h>

DWIDGET_USE_NAMESPACE

class QLabel;
class QHBoxLayout;
class QGridLayout;

// 小日历控件（侧边栏月历），布局参考 dde-calendar(DDE 25) 的 SidebarCalendarWidget
class SidebarCalendarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SidebarCalendarWidget(QWidget *parent = nullptr);

    // 设置一周首日，取值与 constants.h 中的 Week 枚举一致（Sunday = 0 ... Monday = 6）
    void setFirstWeekday(int weekday);
    // 设置选中日期并同步显示月份
    void setDate(const QDate &date);
    QDate selectedDate() const;

signals:
    // 点击了某个日期
    void dateClicked(const QDate &date);
    // 通过上/下月按钮切换了显示月份
    void monthChanged(int year, int month);

private slots:
    void slotNextMonth();
    void slotPreviousMonth();

private:
    void rebuildWeekHeader();
    void updateDateGrid();
    void updateHeaderLabel();

private:
    QDate m_selectedDate;
    QDate m_displayedDate;
    int m_firstWeekDay = 0; // Sunday 首位，默认与主日历一致

    QLabel *m_dateLabel = nullptr;
    DImageButton *m_prevButton = nullptr;
    DImageButton *m_nextButton = nullptr;
    QHBoxLayout *m_weekHeaderLayout = nullptr;
    QGridLayout *m_gridLayout = nullptr;
    QList<QPushButton *> m_dayButtons;
};

// 小日历的日期按键
class SidebarCalendarDayButton : public QPushButton
{
    Q_OBJECT
public:
    explicit SidebarCalendarDayButton(QWidget *parent = nullptr);

    void setDate(const QDate &date);
    QDate date() const;
    void setSelected(bool selected);
    void setInCurrentMonth(bool inCurrentMonth);
    void setToday(bool today);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QDate m_date;
    bool m_selected = false;
    bool m_inCurrentMonth = true;
    bool m_today = false;
};

#endif // SIDEBARCALENDARWIDGET_H
