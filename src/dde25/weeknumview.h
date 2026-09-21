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
 * 移植自 dde-calendar（src/calendar-client/src/widget/weekWidget/weekview.*）。
 * 差异：基类 DWidget 改为 QWidget；去掉触摸手势与键盘代理，周数改用
 *       DDE25::weekNumOfYear（不再依赖 CalendarManager）。
 */

#ifndef WEEKNUMVIEW_H
#define WEEKNUMVIEW_H

#include <QDate>
#include <QDateTime>
#include <QFont>
#include <QList>
#include <QWidget>

#include "constants.h"

class QHBoxLayout;

/**
 * @brief The CWeekNumWidget class
 * 显示前后共 10 周的周数，中间为当前选择周
 */
class CWeekNumWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CWeekNumWidget(QWidget *parent = nullptr);
    ~CWeekNumWidget() override;

    // 设置选择日期并更新
    void setSelectDate(const QDate date);
    // 设置当前日期
    void setCurrent(const QDateTime &dateTime);
    // 设置一周的起始日（影响周数的计算与「选中周」的判断）
    void setFirstWeekDay(Qt::DayOfWeek firstDay);
    // 根据系统主题类型设置颜色
    void setTheMe(int type = 0);

signals:
    void signalsSelectDate(const QDate &date);
    void signalBtnPrev();
    void signalBtnNext();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    bool event(QEvent *e) override;
    bool eventFilter(QObject *o, QEvent *e) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void paintCell(QWidget *cell);
    void setSelectedCell(int index);
    void updateDate();
    void cellClicked(QWidget *cell);

    QList<QWidget *> m_cellList;
    QDate m_selectDate;
    QDate m_days[DDEWeekCalendar::NumWeeksDisplayed];
    int m_selectedCell = 0;
    QFont m_dayNumFont;
    QDateTime m_currentDate;

    QColor m_defaultTextColor = Qt::black;
    QColor m_backgrounddefaultColor = Qt::white;
    QColor m_currentDayTextColor = Qt::white;
    QColor m_backgroundcurrentDayColor = "#0081FF";
    QColor m_fillColor = Qt::white;
    bool m_isFocus = false;
    int m_themetype = 0;
    Qt::DayOfWeek m_firstDay = Qt::Monday;
};

/**
 * @brief The CWeekView class
 * 周数显示条：上一周按钮 + 周数 + 下一周按钮
 */
class CWeekView : public QWidget
{
    Q_OBJECT
public:
    explicit CWeekView(QWidget *parent = nullptr);
    ~CWeekView() override;

    void setSelectDate(const QDate date);
    void setCurrent(const QDateTime &dateTime);
    void setFirstWeekDay(Qt::DayOfWeek firstDay);
    void setTheMe(int type = 0);

signals:
    void signalsSelectDate(const QDate &date);
    void signalBtnPrev();
    void signalBtnNext();

protected:
    void wheelEvent(QWheelEvent *event) override;

private:
    QWidget *m_prevButton = nullptr;
    QWidget *m_nextButton = nullptr;
    CWeekNumWidget *m_weekNumWidget = nullptr;
};

#endif // WEEKNUMVIEW_H
