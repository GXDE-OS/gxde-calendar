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
 * 移植自 dde-calendar（src/calendar-client/src/widget/monthWidget/monthview.*）。
 * 差异：去掉日程浮框 / 新建日程菜单等依赖，仅保留星期标题行 + 6x7 网格。
 */

#ifndef MONTHVIEW_H
#define MONTHVIEW_H

#include <QDate>
#include <QDateTime>
#include <QMap>
#include <QVector>
#include <QWidget>

#include "schedule/dschedule.h"

class CMonthWeekView;
class CMonthGraphicsview;
class QVBoxLayout;

class CMonthView : public QWidget
{
    Q_OBJECT
public:
    explicit CMonthView(QWidget *parent = nullptr);
    ~CMonthView() override;

    // 根据系统主题类型设置颜色
    void setTheMe(int type = 0);
    // 设置每周首日
    void setFirstWeekday(Qt::DayOfWeek weekday);
    // 设置当前时间并刷新 42 格
    void setCurrentDate(const QDate &currentDate);
    // 设置显示农历
    void setLunarVisible(bool visible);
    // 设置班休信息
    void setFestival(const QMap<QDate, int> &festivalInfo);
    // 重新拉取农历并重绘
    void refresh();

signals:
    // 选中某个日期
    void signalsViewSelectDate(QDate date);
    // 滚动相对量
    void signalAngleDelta(int delta);
    // 请求新建日程（右键菜单 / 双击空白格）
    void signalCreateSchedule(QDateTime dateTime);
    // 请求编辑日程（双击日程块 / 右键菜单「编辑」）
    void signalEditSchedule(DSchedule::Ptr schedule);
    // 请求删除日程（右键菜单「删除」）
    void signalDeleteSchedule(DSchedule::Ptr schedule);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    // 计算当前月对应的 42 个日期（按每周首日对齐）
    QVector<QDate> buildShowDates() const;

    CMonthGraphicsview *m_monthGraphicsView = nullptr;
    CMonthWeekView *m_weekIndicator = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;
    QDate m_currentDate;
    Qt::DayOfWeek m_firstWeekDay{Qt::Sunday};
};

#endif // MONTHVIEW_H
