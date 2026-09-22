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
 * graphicsview.* 的网格部分）。
 */

#include "weekgraphicsview.h"

#include "constants.h"
#include "cweekdaybackgrounditem.h"
#include "dde25common.h"
#include "schedulecoormanage.h"
#include "scheduleitem.h"
#include "schedulelayout.h"

#include "schedule/calendarservice.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QScrollBar>
#include <QtMath>

#include <algorithm>

// 参考实现把 CScheduleCoorManage::getDrawRegion 的 type 参数传成 m_viewType。
// 那个枚举是 ALLDayView = 0 / PartTimeView = 1，而网格视图（CGraphicsView）
// 构造时写死 PartTimeView，所以这里恒传 1——跟线上行为一致，不用再去猜 0 的分支。
static const int PartTimeViewType = 1;

CWeekGraphicsView::CWeekGraphicsView(QWidget *parent, ViewPosition viewPos)
    : QGraphicsView(parent)
    , m_viewPos(viewPos)
    , m_Scene(new QGraphicsScene(this))
    , m_coorManage(new CScheduleCoorManage)
{
    setScene(m_Scene);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_LRPen.setStyle(Qt::SolidLine);
    m_LRFlag = true;

    createBackgroundItem();
    setTheMe(DDE25::themeType());
}

CWeekGraphicsView::~CWeekGraphicsView() = default;

void CWeekGraphicsView::createBackgroundItem()
{
    if (m_viewPos == DayPos) {
        CWeekDayBackgroundItem *backgroundItem = new CWeekDayBackgroundItem();
        backgroundItem->setZValue(-1);
        backgroundItem->setBackgroundNum(0);
        m_backgroundItem.append(backgroundItem);
        m_Scene->addItem(backgroundItem);
        return;
    }

    // 周视图：7 列
    for (int i = 0; i < DDEWeekCalendar::AFewDaysofWeek; ++i) {
        CWeekDayBackgroundItem *item = new CWeekDayBackgroundItem();
        item->setZValue(-1);
        if (!m_backgroundItem.isEmpty()) {
            m_backgroundItem.last()->setNextFocusItem(item);
            m_backgroundItem.last()->setRightItem(item);
            item->setLeftItem(m_backgroundItem.last());
        }
        // 设置背景直接分隔符
        item->setDrawDividingLine(true);
        item->setBackgroundNum(i);
        m_backgroundItem.append(item);
        m_Scene->addItem(item);
    }
}

void CWeekGraphicsView::setRange(int w, int h, QDate begindate, QDate enddate, int rightmagin)
{
    if (w <= 0 || h <= 0) {
        return;
    }
    m_beginDate = begindate;
    m_endDate = enddate;
    setBackgroundDate();
    setSceneRect(0, 0, w, h);
    m_coorManage->setRange(w, h, begindate, enddate, rightmagin);
    updateHourPos();
    // 尺寸变了日程块的矩形也得跟着变
    updateInfo();
}

void CWeekGraphicsView::setRange(QDate begin, QDate end)
{
    m_beginDate = begin;
    m_endDate = end;
    setBackgroundDate();
    m_coorManage->setDateRange(begin, end);
    m_Scene->update();
    updateInfo();
}

void CWeekGraphicsView::setBackgroundDate()
{
    for (int i = 0; i < m_backgroundItem.size(); ++i) {
        m_backgroundItem.at(i)->setDate(m_beginDate.addDays(i));
    }
}

void CWeekGraphicsView::setSceneRect(qreal x, qreal y, qreal w, qreal h)
{
    m_Scene->setSceneRect(x, y, w, h);
    const qreal backgroundItemHeight = h;
    const qreal backgroundItemWidth = w / m_backgroundItem.size();

    for (int i = 0; i < m_backgroundItem.size(); ++i) {
        m_backgroundItem.at(i)->setDate(m_beginDate.addDays(i));
        m_backgroundItem.at(i)->setRect(x + backgroundItemWidth * i, 0,
                                        backgroundItemWidth, backgroundItemHeight);
    }
}

void CWeekGraphicsView::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        m_weekColor = "#00429A";
        m_weekColor.setAlphaF(0.05);
        m_gridLineColor = QColor(0, 0, 0, 13);
    } else if (type == 2) {
        m_weekColor = "#4F9BFF";
        m_weekColor.setAlphaF(0.1);
        m_gridLineColor = QColor(255, 255, 255, 10);
    }
    m_LRPen.setColor(m_gridLineColor);

    for (CWeekDayBackgroundItem *item : m_backgroundItem) {
        item->setTheMe(type);
    }

    // 外框背景色，用于遮住右侧竖线（跟随深浅色主题）
    m_outerBorderColor = qApp->palette().color(QPalette::Active, QPalette::Window);

    viewport()->update();
}

void CWeekGraphicsView::setCurrentDate(const QDateTime &currentDate)
{
    m_currentDateTime = currentDate;
    updateHourPos();
}

void CWeekGraphicsView::updateHeight()
{
    m_Scene->update();
    viewport()->update();
}

void CWeekGraphicsView::setInfo(const DSchedule::List &info)
{
    m_scheduleInfo = info;
    updateInfo();
}

void CWeekGraphicsView::clearSchedule()
{
    for (CScheduleItem *item : m_vScheduleItem) {
        if (m_Scene) {
            m_Scene->removeItem(item);
        }
        delete item;
    }
    m_vScheduleItem.clear();
}

void CWeekGraphicsView::addScheduleItem(const DSchedule::Ptr &info, QDate date,
                                        int index, int totalNum, int type)
{
    CScheduleItem *item = new CScheduleItem(
        m_coorManage->getDrawRegion(date, info->dtStart(), info->dtEnd(),
                                    index, totalNum, m_sMaxNum, PartTimeViewType),
        nullptr, type);
    if (type != 0) {
        item->setItemType(CFocusItem::COTHER);
    }
    m_Scene->addItem(item);
    item->setData(info, date, totalNum);
    m_vScheduleItem.append(item);
}

/**
 * @brief CWeekGraphicsView::updateInfo   重排定时日程块
 *
 * 对应参考实现的 CGraphicsView::upDateInfoShow。那边的日程是拖拽过程中
 * 增删改之后增量刷新的（带 DragStatus），本项目只有「整批换掉」一种情形，
 * 所以收成一个清空重排。
 */
void CWeekGraphicsView::updateInfo()
{
    clearSchedule();

    if (!m_beginDate.isValid() || !m_endDate.isValid() || m_scheduleInfo.isEmpty()) {
        if (m_Scene) {
            m_Scene->update();
        }
        return;
    }

    const qint64 count = m_beginDate.daysTo(m_endDate);

    for (int i = 0; i <= count; ++i) {
        const QDate currentDate = m_beginDate.addDays(i);

        // 挑出这一天要画的日程
        DSchedule::List currentInfo;
        for (const DSchedule::Ptr &ptr : m_scheduleInfo) {
            if (ptr.isNull()) {
                continue;
            }
            const qint64 beginoffset = ptr->dtStart().date().daysTo(currentDate);
            const qint64 endoffset = currentDate.daysTo(ptr->dtEnd().date());
            if (beginoffset < 0 || endoffset < 0) {
                continue;
            }
            // 跨天日程在结束日的零点就已经结束了，那一天不画
            if (ptr->dtEnd().date() == currentDate
                && ptr->dtStart().daysTo(ptr->dtEnd()) > 0
                && ptr->dtEnd().time() == QTime(0, 0, 0)) {
                continue;
            }
            currentInfo.append(ptr);
        }
        if (currentInfo.isEmpty()) {
            continue;
        }

        // 时间上重叠的归成一组，组内在一列里并排
        const QVector<DDE25::ScheduleCluster> clusters =
            DDE25::classifyOverlaps(currentInfo, m_minTime);

        for (const DDE25::ScheduleCluster &cluster : clusters) {
            const int tNum = cluster.members.size();

            if (m_viewPos == WeekPos && tNum > m_sMaxNum) {
                // 周视图一列很窄，超过上限就只画前几条，末尾收一个「...」
                for (int n = 0; n < m_sMaxNum - 1; ++n) {
                    addScheduleItem(cluster.members.at(n), currentDate,
                                    n + 1, m_sMaxNum, 0);
                }
                // 占位块借一份日程的样式来画，颜色走的「other」中性色
                int index = m_sMaxNum - 2;
                if (index < 0) {
                    index = 1;
                }
                DSchedule::Ptr detail(cluster.members.at(index)->clone());
                detail->setSummary("1");
                detail->setScheduleTypeID("other");
                addScheduleItem(detail, currentDate, m_sMaxNum, m_sMaxNum, 1);
            } else {
                for (int n = 0; n < tNum; ++n) {
                    addScheduleItem(cluster.members.at(n), currentDate, n + 1, tNum, 0);
                }
            }
        }
    }

    m_Scene->update();
}

void CWeekGraphicsView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    updateHourPos();
}

void CWeekGraphicsView::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx, dy);
    updateHourPos();
}

void CWeekGraphicsView::updateHourPos()
{
    const int viewHeight = viewport()->height();
    if (viewHeight <= 0 || m_Scene->height() <= 0) {
        return;
    }

    m_vLRLarge.clear();
    m_vHours.clear();

    const qreal timeInterval = m_Scene->height() / 24.0;
    const QPointF leftTopRealPos = mapToScene(QPoint(0, 0));
    const QPointF leftBottomRealPos = mapToScene(QPoint(0, viewHeight));

    qreal beginpos = qFloor(leftTopRealPos.y() / timeInterval) * timeInterval;
    if (beginpos < leftTopRealPos.y()) {
        beginpos = (beginpos / timeInterval + 1) * timeInterval;
    }

    for (qreal i = beginpos; i < leftBottomRealPos.y(); i += timeInterval) {
        const QPoint point = mapFromScene(leftBottomRealPos.x(), i);
        m_vLRLarge.append(point.y());
        m_vHours.append(static_cast<int>(qFloor(i / timeInterval + 0.5)));
    }

    const bool showCurrentTime = m_beginDate.isValid() && m_endDate.isValid()
        && m_currentDateTime.date() >= m_beginDate
            && m_currentDateTime.date() <= m_endDate;

    const qreal currentTime = m_currentDateTime.time().msecsSinceStartOfDay() / 86400000.0
                              * m_Scene->height();
    if (showCurrentTime && currentTime > beginpos && currentTime < leftBottomRealPos.y()) {
        m_currentTimeType = 1;
        const QPoint point = mapFromScene(leftBottomRealPos.x(), currentTime);
        m_vLRLarge.append(point.y());
        m_vHours.append(static_cast<int>(qFloor(currentTime / timeInterval + 0.5)));
    } else {
        m_currentTimeType = 0;
    }

    emit signalsPosHours(m_vLRLarge, m_vHours, m_currentTimeType);
    m_Scene->update();
    viewport()->update();
}

DSchedule::Ptr CWeekGraphicsView::scheduleAt(const QPoint &viewPos) const
{
    CScheduleItem *item = dynamic_cast<CScheduleItem *>(itemAt(viewPos));
    // getType() 非 0 的是「还有 N 项」那个占位块（addScheduleItem 的 type 参数）：
    // 它也攥着一条被挤掉的日程，但画出来的是「...」，不该拿它去编辑/删除，
    // 参考实现在右键菜单和双击里都先按 getType() 把这类块挡掉
    if (item == nullptr || item->getType() != 0) {
        return DSchedule::Ptr();
    }
    return item->getData();
}

QDateTime CWeekGraphicsView::scheduleDateTimeAt(const QPoint &viewPos) const
{
    // 没设过范围（m_coorManage 里的日期还是无效值）时不能反查，会算出无意义的时间
    if (!m_coorManage->getBegindate().isValid()) {
        return QDateTime();
    }
    return m_coorManage->getDate(mapToScene(viewPos));
}

void CWeekGraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
{
    // 双击日程块 -> 编辑；双击空白处 -> 按点击位置的时间新建
    const DSchedule::Ptr schedule = scheduleAt(event->pos());
    if (!schedule.isNull()) {
        emit signalEditSchedule(schedule);
        event->accept();
        return;
    }

    const QDateTime dateTime = scheduleDateTimeAt(event->pos());
    if (dateTime.isValid()) {
        emit signalCreateSchedule(dateTime);
        event->accept();
        return;
    }

    QGraphicsView::mouseDoubleClickEvent(event);
}

void CWeekGraphicsView::contextMenuEvent(QContextMenuEvent *event)
{
    // 点在日程块上时菜单给「编辑 / 删除」，空白处才是「新建日程」。
    // 全天区（CAllDayView）覆写了 scheduleAt()，这里同样能取到它的日程块。
    const DSchedule::Ptr schedule = scheduleAt(event->pos());

    const QDateTime dateTime = scheduleDateTimeAt(event->pos());
    if (schedule.isNull() && !dateTime.isValid()) {
        QGraphicsView::contextMenuEvent(event);
        return;
    }

    popupMenu(event->globalPos(), dateTime, schedule);
    event->accept();
}

void CWeekGraphicsView::popupMenu(const QPoint &globalPos, const QDateTime &dateTime,
                                  const DSchedule::Ptr &schedule)
{
    QMenu menu(this);

    if (!schedule.isNull()) {
        // 菜单项与参考实现一致：编辑在前、删除在后
        QAction *editAction = menu.addAction(tr("Edit"), this, [this, schedule] {
            emit signalEditSchedule(schedule);
        });
        QAction *deleteAction = menu.addAction(tr("Delete"), this, [this, schedule] {
            emit signalDeleteSchedule(schedule);
        });
        // 只读日历（ICS 订阅）的日程改不了（见 isScheduleEditable 的说明），
        // 菜单上直接把「编辑」置灰，别让人点开一个全灰的表单
        editAction->setEnabled(CalendarService::instance()->isScheduleEditable(schedule));
        // 只读日历（ICS 订阅、节假日）把「删除」置灰，参考实现同样处理
        deleteAction->setEnabled(CalendarService::instance()->isScheduleDeletable(schedule));
    } else {
        menu.addAction(tr("New Schedule"), this, [this, dateTime] {
            emit signalCreateSchedule(dateTime);
        });
    }

    menu.exec(globalPos);
}

void CWeekGraphicsView::paintEvent(QPaintEvent *event)
{
    // 先画背景与网格线，再交给 QGraphicsView 绘制 scene（包含各列背景项）
    QPainter painter(viewport());
    const int t_width = viewport()->width() + 2;

    if (m_LRFlag && !m_vLRLarge.isEmpty()) {
        painter.save();
        painter.setPen(m_LRPen);
        const int count = m_currentTimeType == 0 ? m_vLRLarge.size() : m_vLRLarge.size() - 1;
        for (int i = 0; i < count; ++i) {
            painter.drawLine(QPoint(0, m_vLRLarge[i] - 1), QPoint(t_width, m_vLRLarge[i] - 1));
        }
        painter.restore();

        if (m_currentTimeType == 1) {
            // 当前时刻线用高亮色画在最后一根
            painter.save();
            QPen pen = m_LRPen;
            pen.setColor(m_currenttimecolor);
            painter.setPen(pen);
            const int index = m_vLRLarge.size() - 1;
            painter.drawLine(QPoint(0, m_vLRLarge[index] - 1), QPoint(t_width, m_vLRLarge[index] - 1));
            painter.restore();
        }
    }

    painter.end();

    QGraphicsView::paintEvent(event);
}
