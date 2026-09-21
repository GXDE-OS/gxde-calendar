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
 * 移植自 dde-calendar（src/calendar-client/src/widget/dayWidget/daymonthview.*）。
 * 差异：
 *   1. DIconButton/CTodayButton/DHorizontalLine 换成 QToolButton/QPushButton/QFrame
 *      （本项目未引入 DtkWidget 头文件路径）。
 *   2. 黄历信息改用 gxde 农历服务已有的 CaLunarDayInfo；该服务不提供「宜/忌」数据，
 *      因此原实现的 m_yiLabel/m_jiLabel 及其下方的宜/忌区域整体不移植
 *      （dayhuangliview.* 与两个宜/忌图标也未引入）。
 *   3. 去掉搜索态与拖拽新建日程（本阶段无日程数据层）。
 */

#ifndef DAYMONTHVIEW_H
#define DAYMONTHVIEW_H

#include "calendardbus.h"
#include "customframe.h"

#include <QDate>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QList>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

class CDayMonthWidget;
class CWeekWidget;
class QToolButton;
class QPushButton;

/**
 * @brief The CDayMonthView class
 * 日视图右侧栏：迷你月历 + 大号日期 + 星期/年月/农历干支
 */
class CDayMonthView : public CustomFrame
{
    Q_OBJECT

public:
    explicit CDayMonthView(QWidget *parent = nullptr);
    ~CDayMonthView() override;

    // 设置显示时间、选择时间和当前时间
    void setShowDate(const QVector<QDate> &showDate, const QDate &selectDate, const QDate &currentDate);
    void setLunarVisible(bool visible);
    // 设置一周起始日（影响星期名行的排布）
    void setFirstWeekDay(Qt::DayOfWeek firstDay);
    void setTheMe(int type = 0);
    // 设置黄历/农历信息
    void setLunarInfo(const CaLunarDayInfo &lunarInfo);
    // 设置迷你月历各天是否有日程（Stage 2 使用）
    void setHasScheduleFlag(const QVector<bool> &hasScheduleFlag);

signals:
    // 选择时间改变信号
    void signalChangeSelectDate(const QDate &date);

public slots:
    // 上一个月
    void slotprev();
    // 下一个月
    void slotnext();
    // 返回当前时间
    void slottoday();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *e) override;

private:
    void initUI();
    void initConnection();
    void updateDateShow();
    void updateDateLunarDay();
    void changeSelectDate(const QDate &date);

    QToolButton *m_prevButton = nullptr;
    QToolButton *m_nextButton = nullptr;
    QPushButton *m_today = nullptr;
    CustomFrame *m_currentMouth = nullptr;
    CustomFrame *m_currentDay = nullptr;
    CustomFrame *m_currentYear = nullptr;
    CustomFrame *m_currentWeek = nullptr;
    CustomFrame *m_currentLuna = nullptr;
    QDate m_selectDate;
    QDate m_currentDate;
    QColor m_backgroundCircleColor = "#2ca7f8";
    QColor m_weekendsTextColor = Qt::black;
    QColor m_festivalTextColor = Qt::black;
    Qt::DayOfWeek m_firstWeekDay = Qt::Monday;
    bool m_huanglistate = true;
    QVBoxLayout *m_hhLayout = nullptr;
    QVBoxLayout *m_upLayout = nullptr;
    QFrame *m_splitline = nullptr;
    QVBoxLayout *m_yiDownLayout = nullptr;
    QVBoxLayout *m_jiDownLayout = nullptr;
    QStringList m_weeklist;
    CaLunarDayInfo m_lunarInfo;
    CDayMonthWidget *m_dayMonthWidget = nullptr;
    const int m_radius = 8;

    CWeekWidget *m_weekWidget = nullptr; // 星期名显示区域
};

/**
 * @brief The CDayMonthWidget class
 * 日视图侧栏的 6×7 迷你月历
 */
class CDayMonthWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CDayMonthWidget(QWidget *parent = nullptr);
    ~CDayMonthWidget() override;

    void setTheMe(int type = 0);
    // 设置显示时间、选择时间和当前时间
    void setShowDate(const QVector<QDate> &showDate, const QDate &selectDate, const QDate &currentDate);
    void setHasScheduleFlag(const QVector<bool> &hasScheduleFlag);

signals:
    // 选择时间改变信号
    void signalChangeSelectDate(const QDate &date);

public slots:
    // 点击月显示时间修改选择时间
    void cellClicked(QWidget *cell);
    // 设置选择项修改选择时间
    void setSelectedCell(int index);

protected:
    bool eventFilter(QObject *o, QEvent *e) override;
    void resizeEvent(QResizeEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    const QString getCellDayNum(int pos);
    const QDate getCellDate(int pos);
    void paintCell(QWidget *cell);

    QList<QWidget *> m_cellList;
    QVector<QDate> m_showDays;
    int m_selectedCell = 0;
    QDate m_selectDate;
    QDate m_currentDate;
    QGridLayout *m_gridLayout = nullptr;
    QVector<bool> m_vlineflag; // 日程标识

    QColor m_selectedTextColor = Qt::white;
    QColor m_currentDayTextColor = "#2ca7f8";
    QColor m_defaultTextColor = Qt::black;
    QColor m_notCurrentTextColor = "#b2b2b2";
    QColor m_ceventColor = "#FF5D00";
    QFont m_dayNumFont;
    bool m_isFocus = false;
    QPoint m_startPos;
};

#endif // DAYMONTHVIEW_H
