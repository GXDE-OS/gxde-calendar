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
 * 移植自 dde-calendar（src/calendar-client/src/widget/weekWidget/weekheadview.*）。
 * 差异：基类由 DWidget 改为 QWidget；农历信息改用 DDE25::LunarCache（复用黄历 DBus
 *       服务），不再依赖 huangliData/dbusdatastruct.h；去掉触摸手势处理。
 */

#ifndef WEEKHEADVIEW_H
#define WEEKHEADVIEW_H

#include <QDate>
#include <QFont>
#include <QList>
#include <QVector>
#include <QWidget>

class CustomFrame;

/**
 * @brief The CWeekHeadView class
 * 周视图日期模块显示
 */
class CWeekHeadView : public QWidget
{
    Q_OBJECT
public:
    enum ShowState {
        ShowLunar = 0x01,
        ShowLunarFestivalHighlight = 0x02,
        Normal = ShowLunar | ShowLunarFestivalHighlight,
    };

    explicit CWeekHeadView(QWidget *parent = nullptr);
    ~CWeekHeadView() override;

    // 根据系统主题类型设置颜色
    void setTheMe(int type = 0);
    // 设置一周的时间
    void setWeekDay(QVector<QDate> vDays, const QDate &selectDate);
    // 设置是否显示阴历信息
    void setLunarVisible(bool visible);
    // 通知农历数据已更新，需要重绘
    void updateLunar();

signals:
    // 选择日期的信号
    void signalsViewSelectDate(QDate date);
    // 发送滚动信号滚动相对量
    void signalAngleDelta(int delta);

protected:
    void wheelEvent(QWheelEvent *e) override;
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    // 根据索引值获取当天是在一个月中的第几天
    QString getCellDayNum(int pos);
    // 根据索引获取当天的日期
    QDate getCellDate(int pos);
    // 根据索引值获取当天的阴历信息
    QString getLunar(int pos);
    // 绘制周信息
    void paintCell(QWidget *cell);

    QList<QWidget *> m_cellList;
    CustomFrame *m_monthLabel = nullptr;
    QVector<QDate> m_days;
    ShowState m_showState = Normal;
    int m_selectedCell = 0;

    QFont m_dayNumFont;
    QFont m_monthFont;

    QColor m_backgroundCircleColor = "#2ca7f8";
    QColor m_backgroundShowColor = "#2CA7F8";
    QColor m_defaultTextColor = "#6F6F6F";
    QColor m_currentDayTextColor = "#FFFFFF";
    QColor m_defaultLunarColor = "#898989";
    QColor m_weekendsTextColor = "#0887FF";
    QColor m_currentMonthColor = "#000000";
    QColor m_backgroundColor = "#E6EEF2";
    // 周六周日背景色
    QColor m_Background_Weekend_Color = "#00429A";
    QColor m_dividingLineColor = QColor(0, 0, 0, 26);
    QColor m_solofestivalLunarColor = "#4DFF7272";
    int m_themetype = 1;
    const int m_radius = 8;
};

#endif // WEEKHEADVIEW_H
