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
 * This file is ported from DDE calendar tag 6.6.0
 * Minimal modification is applied to make the class build against
 * DTK2Widget-Qt6.
 * ----------------------------------------------------------------------------
 * 移植自 dde-calendar（src/calendar-client/src/view/monthgraphiview.*）。
 *
 * dde-calendar 的 CMonthGraphicsview 继承自 DragInfoGraphicsView（拖拽建日程 /
 * 键盘导航 / 右键菜单）。这里直接继承 QGraphicsView，保留 42 格布局、农历、
 * 班休、今日圆点、圆角等绘制逻辑，只补上「双击日程块编辑 / 右键或双击空白格
 * 新建」两个入口，外加日程块的右键菜单（编辑 / 删除，对齐参考实现
 * draginfographicsview.cpp 的右键菜单；确认弹窗与数据层调用在 CalendarWindow
 * 里，视图只往外发信号），拖拽改期与键盘导航留到后续阶段接入。
 *
 * 差异：参考实现的新建时间取自 CScheduleCoorManage::getDate(scenePos)，但它的
 * 月视图从不调用 setRange（只有周/日视图在 cweekdaygraphicsview.cpp 里设过），
 * 那里会除零算出无意义的时间。这里改用被点格子的日期、时刻取 00:00，不复制
 * 参考实现的这个缺陷。
 *
 * 差异：resizeEvent() 里补了一次 updateScheduleItems()。参考实现的日程数据是
 *       异步（DBus）取回来的，首次 setScheduleInfo 落在窗口布局之后；本项目
 *       同步查库，构造期的 refresh() 在视图还是默认尺寸时就排过版，算出来 0 块，
 *       不在 resize 时重排的话重开应用月视图是空的（详见 resizeEvent 注释）。
 */

#ifndef MONTHGRAPHICSVIEW_H
#define MONTHGRAPHICSVIEW_H

#include <QColor>
#include <QDate>
#include <QDateTime>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMap>
#include <QVector>

#include "schedule/dschedule.h"

class CMonthDayItem;
class CMonthScheduleItem;

class CMonthGraphicsview : public QGraphicsView
{
    Q_OBJECT
public:
    explicit CMonthGraphicsview(QWidget *parent = nullptr);
    ~CMonthGraphicsview() override;

    void setTheMe(int type = 0);
    // 设置 42 格对应的日期
    void setDate(const QVector<QDate> &showDate);
    // 设置班休信息
    void setFestival(const QMap<QDate, int> &festivalInfo);
    // 设置是否显示农历信息
    void setLunarVisible(bool visible);
    // 设置 42 格里的日程（按天分组，数据层已经展开好重复日程）
    void setScheduleInfo(const QMap<QDate, DSchedule::List> &scheduleInfo);

signals:
    void signalsViewSelectDate(QDate date);
    // 滚动相对量（参考实现 CMonthGraphicsview::wheelEvent）
    void signalAngleDelta(int delta);
    // 请求新建日程（右键菜单 / 双击空白格），携带要新建的日期与时刻
    void signalCreateSchedule(QDateTime dateTime);
    // 请求编辑日程（双击日程块 / 右键菜单「编辑」）
    void signalEditSchedule(DSchedule::Ptr schedule);
    // 请求删除日程（右键菜单「删除」，确认弹窗与数据层调用在 CalendarWindow 里）
    void signalDeleteSchedule(DSchedule::Ptr schedule);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    // 场景坐标落在哪个格子上，不在任何格子上时返回无效日期
    QDate dateAt(const QPointF &scenePos) const;
    // 视口坐标下的日程块，不是日程块时返回 nullptr
    CMonthScheduleItem *scheduleItemAt(const QPoint &viewPos) const;
    // 右键菜单：schedule 非空（点中日程块）给「编辑 / 删除」，对齐参考实现
    // draginfographicsview.cpp 的右键菜单；为空（点中空白格）给「新建日程」
    void popupMenu(const QPoint &globalPos, const QDate &date, const DSchedule::Ptr &schedule);
    void applyHolidayStatus();

    void updateSize();
    // 从 DDE25::LunarCache 读取当前 42 格对应的农历文本
    void updateLunar();
    // 按当前日期与格子尺寸重排日程块
    void updateScheduleItems();
    void clearScheduleItems();

    QGraphicsScene *m_Scene = nullptr;
    QVector<CMonthDayItem *> m_DayItem;
    QMap<QDate, int> m_festivallist;
    int m_themetype = 0;

    // 当前 42 格对应的日期，重排日程块时要拿它当起点
    QVector<QDate> m_showDates;
    QMap<QDate, DSchedule::List> m_scheduleInfo;
    QVector<QGraphicsItem *> m_scheduleItems;

    // 左下/右下圆角
    qreal m_radius{16};
    bool m_leftShowRadius{false};
    bool m_rightShowRadius{false};
    QColor m_outerBorderColor;
};

#endif // MONTHGRAPHICSVIEW_H
