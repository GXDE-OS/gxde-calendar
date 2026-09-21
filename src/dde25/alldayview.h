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
 * 移植自 dde-calendar（src/calendar-client/src/view/alldayeventview.* 的绘制部分）。
 *
 * 时间网格上方的全天日程条：把全天日程按「能不重叠就不重叠」排成若干行，
 * 行数决定这一条的高度，高度反过来通知外层挪「ALL DAY」标签和分隔线。
 * 参考实现继承 CWeekDayGraphicsview，这里复用已经移植好的 CWeekGraphicsView。
 */

#ifndef ALLDAYVIEW_H
#define ALLDAYVIEW_H

#include "schedule/dschedule.h"
#include "weekgraphicsview.h"

#include <QVector>

class CAllDayScheduleItem;

class CAllDayView : public CWeekGraphicsView
{
    Q_OBJECT
public:
    explicit CAllDayView(QWidget *parent = nullptr, ViewPosition viewPos = WeekPos);
    ~CAllDayView() override;

    // 设置全天日程（扁平列表，内部按行打包）
    void setAllDayInfo(const DSchedule::List &info);

    // 全天条的高度由行数决定，h 参数忽略
    void setRange(int w, int h, QDate begindate, QDate enddate, int rightmagin = 0) override;
    void setRange(QDate begin, QDate end) override;
    void setTheMe(int type = 0) override;
    void clearSchedule() override;
    // 按当前日期与尺寸重排全天日程
    void updateInfo() override;

signals:
    // 全天区应有的高度，外层据此放「ALL DAY」标签并画分隔线
    void signalUpdatePaint(int topM);

protected:
    void paintEvent(QPaintEvent *event) override;
    void changeEvent(QEvent *event) override;

    // 全天区的日程块是 CAllDayScheduleItem（不是 CScheduleItem），要单独取
    DSchedule::Ptr scheduleAt(const QPoint &viewPos) const override;
    // 全天区的高度不是 24 小时，CScheduleCoorManage::getDate 算出来的时刻没有
    // 意义，这里只取日期，时刻固定 00:00
    QDateTime scheduleDateTimeAt(const QPoint &viewPos) const override;

private:
    void updateDateShow();
    void createItemWidget(int index);
    void clearItems();
    // 行高跟随字体，参考实现的 updateItemHeightByFontSize
    void updateItemHeightByFontSize();

    // 待排布的全天日程
    DSchedule::List m_allDayInfo;
    // 打包后的每一行放哪些日程，行号即第几行
    QVector<DSchedule::List> m_vlistData;
    QVector<CAllDayScheduleItem *> m_baseShowItem;
    int m_itemHeight = 22;
    int m_width = 0;

    QColor m_dividingLineColor = QColor(0, 0, 0, 13);
};

#endif // ALLDAYVIEW_H
