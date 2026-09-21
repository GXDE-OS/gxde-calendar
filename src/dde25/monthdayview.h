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
 * 移植自 dde-calendar（src/calendar-client/src/widget/monthWidget/monthdayview.*）。
 * 差异：
 *   1. 外壳基类由 DFrame 改为 QWidget，自己画圆角外框。配色不取参考实现的
 *      纯白（在 DDE25 的纯白页面上会看不见），而取 gxde-file-manager 地址栏
 *      的样子：浅色 #FFFFFF + 1px rgba(0,0,0,0.12) 描边，深色 #343434 无描边
 *      （见 gxde-file-manager-lib/interfaces/dfmcrumbbar.cpp 的 paintEvent 与
 *      themes/{light,dark}/config.ini 的 DFMCrumbBar / CrumbBar.BorderLine）。
 *   2. 去掉触摸手势（touchGestureOperation）与搜索态。
 */

#ifndef MONTHDAYVIEW_H
#define MONTHDAYVIEW_H

#include <QColor>
#include <QDate>
#include <QFont>
#include <QRectF>
#include <QVector>
#include <QWidget>

class CMonthRect;
class CMonthWidget;

/**
 * @brief 月份滚轮：一块圆角外框，里面一排 12 个月份（以选中月为中心前后各 5 个月）
 */
class CMonthDayView : public QWidget
{
    Q_OBJECT
public:
    explicit CMonthDayView(QWidget *parent = nullptr);

    // 设置选择时间（12 个月以它为中心重排）
    void setSelectDate(const QDate &date);
    // 设置主题颜色
    void setTheMe(int type = 0);

signals:
    // 切换月份修改选择时间
    void signalsSelectDate(QDate date);
    // 滚动相对量
    void signalAngleDelta(int delta);

protected:
    void wheelEvent(QWheelEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

private:
    CMonthWidget *m_monthWidget = nullptr;
    QDate m_selectDate;
    QDate m_days[12];
    QColor m_frameColor = "#FFFFFF";
    QColor m_borderColor = QColor(0, 0, 0, 30);
    int m_radius = 8;
};

/**
 * @brief 月份滚轮里的 12 个格子
 */
class CMonthWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CMonthWidget(QWidget *parent = nullptr);
    ~CMonthWidget() override;

    void setDate(const QDate date[12]);

signals:
    // 切换月份修改选择时间
    void signalsSelectDate(QDate date);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    // 更新格子大小
    void updateSize();
    // 根据选择时间重排 12 个月
    void updateShowDate(const QDate &selectDate);
    // 获取鼠标点击的区域编号
    int getMousePosItem(const QPointF &pos);
    void mousePress(const QPoint &point);

    QVector<CMonthRect *> m_MonthItem;
    QDate m_days[12];
    bool m_isFocus = false;
};

/**
 * @brief 月份滚轮里的单个月份
 */
class CMonthRect
{
public:
    CMonthRect() = default;

    void setDate(const QDate &date);
    QDate getDate() const;
    QRectF rect() const;
    void setRect(const QRectF &rect);
    void setRect(qreal x, qreal y, qreal w, qreal h);
    void paintItem(QPainter *painter, const QRectF &rect, bool drawFocus = false);

    static void setTheMe(int type);
    static void setSelectRect(CMonthRect *selectRect);
    static CMonthRect *getSelectRect();

private:
    QRectF m_rect;
    QDate m_Date;

    static int m_themetype;
    static QColor m_defaultTextColor;
    // 选中月圆点里的文字色
    static QColor m_currentDayTextColor;
    // 当前月的文字色（参考实现里复用了这个名字）
    static QColor m_backgroundcurrentDayColor;
    static QFont m_dayNumFont;
    static CMonthRect *m_SelectRect;
};

#endif // MONTHDAYVIEW_H
