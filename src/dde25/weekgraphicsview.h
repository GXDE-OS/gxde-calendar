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
 * 移植自 dde-calendar（src/calendar-client/src/view/cweekdaygraphicsview.* 与
 * graphicsview.*）。差异：不继承 DragInfoGraphicsView（拖拽改期、长按、右键菜单
 * 那一整套交互都没移植），直接继承 QGraphicsView，只保留背景网格 + 定时日程块。
 */

#ifndef WEEKGRAPHICSVIEW_H
#define WEEKGRAPHICSVIEW_H

#include "schedule/dschedule.h"

#include <QDate>
#include <QDateTime>
#include <QGraphicsView>
#include <QPen>
#include <QVector>

#include <memory>

class CWeekDayBackgroundItem;
class CScheduleCoorManage;
class CScheduleItem;

/**
 * @brief The CWeekGraphicsView class
 * 周/日视图的非全天区域：7 列（日视图 1 列）背景 + 整点网格 + 定时日程块。
 */
class CWeekGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    enum ViewPosition {
        WeekPos,
        DayPos
    };

    explicit CWeekGraphicsView(QWidget *parent = nullptr, ViewPosition viewPos = WeekPos);
    ~CWeekGraphicsView() override;

    // 设置场景尺寸与日期范围。h 为场景高度（= 24 小时的总像素高）。
    virtual void setRange(int w, int h, QDate begindate, QDate enddate, int rightmagin = 0);
    virtual void setRange(QDate begin, QDate end);
    virtual void setTheMe(int type = 0);
    // 设置当前时间，用于绘制当前时刻线
    void setCurrentDate(const QDateTime &currentDate);
    // 内容尺寸变化后刷新网格
    void updateHeight();

    // 设置要绘制的定时日程（非全天，扁平列表，内部按天分发）
    void setInfo(const DSchedule::List &info);
    // 按当前尺寸与日期重排日程块
    virtual void updateInfo();
    // 清掉所有日程块
    virtual void clearSchedule();
    // 一列里最多并排显示几条（周视图用，超过的收成一个「...」）
    void setMaxNum(int maxnum) { m_sMaxNum = maxnum; }
    // 最短时长对应的秒数，用来把过短的日程撑到看得见
    void setMinTime(int minTime) { m_minTime = minTime; }
    CScheduleCoorManage *getCoorManage() const { return m_coorManage.get(); }

signals:
    // 整点位置（视口坐标）与对应小时，供外层的左侧时间栏绘制文字
    void signalsPosHours(QVector<int> vPos, QVector<int> vHours, int currentTimeType);
    // 请求新建日程（右键菜单 / 双击空白处），携带点击位置对应的时间
    void signalCreateSchedule(QDateTime dateTime);
    // 请求编辑日程（双击日程块）
    void signalEditSchedule(DSchedule::Ptr schedule);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void scrollContentsBy(int dx, int dy) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

    // 视口坐标下日程块对应的日程，不在日程块上时返回空
    virtual DSchedule::Ptr scheduleAt(const QPoint &viewPos) const;
    // 视口坐标对应的时间，超出已设范围时返回无效值（右键新建日程用）
    virtual QDateTime scheduleDateTimeAt(const QPoint &viewPos) const;
    // 右键菜单：目前只有「新建日程」，编辑/删除在后续阶段接入
    void popupMenu(const QPoint &globalPos, const QDateTime &dateTime);

    void createBackgroundItem();
    // 按 m_beginDate 给各列背景项设日期
    void setBackgroundDate();
    // 设场景尺寸，同时把 7 列背景项铺满，子类（全天区）改高度也要走这里
    void setSceneRect(qreal x, qreal y, qreal w, qreal h);
    // 重新计算整点位置并通知外层
    void updateHourPos();
    // 加一块日程，index 从 1 开始，type 非 0 表示「还有更多」的占位块
    void addScheduleItem(const DSchedule::Ptr &info, QDate date, int index, int totalNum, int type);

    ViewPosition m_viewPos;
    QGraphicsScene *m_Scene = nullptr;
    QVector<CWeekDayBackgroundItem *> m_backgroundItem;
    QDate m_beginDate;
    QDate m_endDate;

    // 时间区间 ↔ 视图矩形的换算（整棵移植自参考实现的 CScheduleCoorManage）
    std::unique_ptr<CScheduleCoorManage> m_coorManage;
    // 一列里最多并排几条；周视图按列宽算出来，日视图不限
    int m_sMaxNum = 4;
    // 最短时长（秒），把过短的日程撑到看得见
    int m_minTime = 0;
    int m_rightmagin = 0;
    // 整点横线开关，全天区关掉（它没有时间网格）
    bool m_LRFlag = true;

private:
    // 待绘制的定时日程（扁平列表，每天在自己那一列里分发）
    DSchedule::List m_scheduleInfo;
    QVector<CScheduleItem *> m_vScheduleItem;

    QPen m_LRPen;
    QVector<int> m_vLRLarge; // 整点线的视口 y 坐标
    QVector<int> m_vHours;
    int m_currentTimeType = 0;
    QDateTime m_currentDateTime = QDateTime::currentDateTime();
    QColor m_gridLineColor = QColor(0, 0, 0, 13);
    QColor m_weekColor = "#00429A";
    QColor m_currenttimecolor = "#F74444";
    QColor m_outerBorderColor; // 外框背景色，用于遮住右侧竖线
};

#endif // WEEKGRAPHICSVIEW_H
