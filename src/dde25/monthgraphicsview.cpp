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
 */

#include "monthgraphicsview.h"

#include "cmonthdayitem.h"
#include "constants.h"
#include "dde25common.h"
#include "cmonthscheduleitem.h"
#include "cmonthschedulenumitem.h"
#include "schedulelayout.h"

#include "schedule/calendarservice.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>

CMonthGraphicsview::CMonthGraphicsview(QWidget *parent)
    : QGraphicsView(parent)
{
    // 显示左右下角圆角
    m_leftShowRadius = true;
    m_rightShowRadius = true;

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    m_Scene = new QGraphicsScene(this);
    setScene(m_Scene);

    for (int i = 0; i < DDEMonthCalendar::ItemSizeOfMonthDay; ++i) {
        CMonthDayItem *item = new CMonthDayItem();
        item->setZValue(-1);
        if (!m_DayItem.isEmpty()) {
            // 设置对应左右和下一个的关系
            m_DayItem.last()->setNextFocusItem(item);
            m_DayItem.last()->setRightItem(item);
            item->setLeftItem(m_DayItem.last());
        }
        const int upNum = i - DDEMonthCalendar::AFewDaysOfWeek;
        if (upNum >= 0) {
            m_DayItem.at(upNum)->setDownItem(item);
            item->setUpItem(m_DayItem.at(upNum));
        }
        item->setBackgroundNum(i);
        m_DayItem.append(item);
        m_Scene->addItem(item);
    }

    updateSize();
}

CMonthGraphicsview::~CMonthGraphicsview()
{
    m_DayItem.clear();
}

void CMonthGraphicsview::setTheMe(int type)
{
    m_themetype = type;

    // 外框背景色，用于绘制左下/右下圆角的补角（取应用调色板，跟随深浅色主题）
    m_outerBorderColor = qApp->palette().color(QPalette::Active, QPalette::Window);

    for (CMonthDayItem *item : m_DayItem) {
        item->setTheMe(type);
    }
    viewport()->update();
}

void CMonthGraphicsview::setDate(const QVector<QDate> &showDate)
{
    Q_ASSERT(showDate.size() == DDEMonthCalendar::ItemSizeOfMonthDay);
    if (showDate.isEmpty()) {
        return;
    }

    m_showDates = showDate;

    const int currentMonth = showDate.at(0).day() != 1
        ? showDate.at(0).addMonths(1).month()
        : showDate.at(0).month();

    for (int i = 0; i < m_DayItem.size(); ++i) {
        m_DayItem.at(i)->setDate(showDate.at(i));
        m_DayItem.at(i)->setCurrentMonth(showDate.at(i).month() == currentMonth);
    }

    updateLunar();
    updateSize();
    updateScheduleItems();
    scene()->update();
}

void CMonthGraphicsview::setScheduleInfo(const QMap<QDate, DSchedule::List> &scheduleInfo)
{
    m_scheduleInfo = scheduleInfo;
    updateScheduleItems();
    scene()->update();
}

void CMonthGraphicsview::setFestival(const QMap<QDate, int> &festivalInfo)
{
    m_festivallist = festivalInfo;
    for (CMonthDayItem *item : m_DayItem) {
        item->setStatus(static_cast<CMonthDayItem::HolidayStatus>(m_festivallist[item->getDate()]));
    }
    if (scene()) {
        scene()->update();
    }
}

void CMonthGraphicsview::setLunarVisible(bool visible)
{
    CMonthDayItem::m_LunarVisible = visible;
    updateLunar();
    if (scene()) {
        scene()->update();
    }
}

void CMonthGraphicsview::updateSize()
{
    // 场景的大小和位置
    const QRectF sceneRect(0, 0, viewport()->rect().width(), viewport()->rect().height());
    m_Scene->setSceneRect(sceneRect);

    const qreal w = m_Scene->width() / DDEMonthCalendar::AFewDaysOfWeek;
    const qreal h = m_Scene->height() / DDEMonthCalendar::LinesNumOfMonth;

    for (int i = 0; i < m_DayItem.size(); ++i) {
        const int hOffset = i / DDEMonthCalendar::AFewDaysOfWeek;
        const int wOffset = i % DDEMonthCalendar::AFewDaysOfWeek;
        m_DayItem.at(i)->setRect(QRectF(w * wOffset, h * hOffset, w, h));
    }
}

void CMonthGraphicsview::updateLunar()
{
    DDE25::LunarCache *cache = DDE25::LunarCache::instance();

    for (CMonthDayItem *item : m_DayItem) {
        item->setLunar(cache->lunarText(item->getDate()));
    }
}

void CMonthGraphicsview::clearScheduleItems()
{
    for (QGraphicsItem *item : m_scheduleItems) {
        if (scene()) {
            scene()->removeItem(item);
        }
        delete item;
    }
    m_scheduleItems.clear();
}

void CMonthGraphicsview::updateScheduleItems()
{
    clearScheduleItems();

    if (!scene() || m_showDates.isEmpty()
        || m_showDates.size() != DDEMonthCalendar::ItemSizeOfMonthDay) {
        return;
    }

    const int sceneW = static_cast<int>(m_Scene->width());
    const int sceneH = static_cast<int>(m_Scene->height());
    if (sceneW <= 0 || sceneH <= 0) {
        return;
    }

    const int itemHeight = DDEMonthCalendar::MonthScheduleItemHeight;
    const QVector<DDE25::MonthBlock> blocks =
        DDE25::layoutMonthBlocks(m_scheduleInfo, m_showDates.first(), sceneW, sceneH, itemHeight);

    for (const DDE25::MonthBlock &block : blocks) {
        if (block.isMore) {
            CMonthScheduleNumItem *numItem = new CMonthScheduleNumItem();
            // 背景透明、文字用中性灰，跟参考实现一致
            QColor gradient("#000000");
            gradient.setAlphaF(0.00);
            numItem->setColor(gradient, gradient);
            QColor textColor(DDE25::themeType() == 2 ? "#FFFFFF" : "#5E5E5E");
            textColor.setAlphaF(0.9);
            numItem->setText(textColor, font());
            numItem->setData(block.moreCount);
            numItem->setRect(block.rect);
            numItem->setDate(block.date);
            m_Scene->addItem(numItem);
            m_scheduleItems.append(numItem);
            continue;
        }

        if (block.schedule.isNull()) {
            continue;
        }

        CMonthScheduleItem *item = new CMonthScheduleItem(block.rect);
        item->setData(block.schedule);
        item->setDate(block.date);
        m_Scene->addItem(item);
        m_scheduleItems.append(item);
    }
}

void CMonthGraphicsview::wheelEvent(QWheelEvent *event)
{
    // 参考实现：滚动为上下则发送信号用于翻月
    if (event->angleDelta().y() != 0) {
        emit signalAngleDelta(event->angleDelta().y());
    }
}

void CMonthGraphicsview::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    updateSize();
    // 日程块的矩形是按场景尺寸算的，尺寸一变就得重排。
    // 启动时视图还拿着 viewport 的默认尺寸（100x30），构造期的 refresh() 在这个
    // 尺寸下算出来 0 块（每格减掉日期头之后放不下一行日程），而月视图的日程数据
    // 是同步查出来的、不会再自己来一次；不在 resize 时补这一下的话，重开应用后
    // 月视图就一条日程都不显示，要等切月或增删日程触发的下一次 refresh() 才出来。
    // 参考实现是异步（DBus）取数据，首次 setScheduleInfo 落在窗口布局完成之后，
    // 所以那边不需要这一步。
    updateScheduleItems();
}

void CMonthGraphicsview::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);

    // 绘制圆角效果：用外框背景色补掉方角
    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_outerBorderColor);

    // 左下角补角
    if (m_leftShowRadius) {
        QPainterPath leftPath;
        leftPath.moveTo(0, height() - m_radius);
        leftPath.arcTo(0, height() - m_radius * 2, m_radius * 2, m_radius * 2, 180, 90);
        leftPath.lineTo(0, height());
        leftPath.lineTo(0, height() - m_radius);
        painter.drawPath(leftPath);
    }

    // 右下角补角
    if (m_rightShowRadius) {
        QPainterPath rightPath;
        rightPath.moveTo(width() - m_radius, height());
        rightPath.arcTo(width() - m_radius * 2, height() - m_radius * 2, m_radius * 2, m_radius * 2, 270, 90);
        rightPath.lineTo(width(), height());
        rightPath.lineTo(width() - m_radius, height());
        painter.drawPath(rightPath);
    }
}

void CMonthGraphicsview::mousePressEvent(QMouseEvent *event)
{
    const QDate date = dateAt(mapToScene(event->pos()));
    if (date.isValid()) {
        emit signalsViewSelectDate(date);
    }
    QGraphicsView::mousePressEvent(event);
}

void CMonthGraphicsview::mouseDoubleClickEvent(QMouseEvent *event)
{
    // 双击日程块 -> 编辑；双击空白格 -> 新建
    if (CMonthScheduleItem *item = scheduleItemAt(event->pos())) {
        emit signalEditSchedule(item->getData());
        event->accept();
        return;
    }

    const QDate date = dateAt(mapToScene(event->pos()));
    if (date.isValid()) {
        emit signalCreateSchedule(QDateTime(date, QTime(0, 0)));
        event->accept();
        return;
    }

    QGraphicsView::mouseDoubleClickEvent(event);
}

void CMonthGraphicsview::contextMenuEvent(QContextMenuEvent *event)
{
    // 点在日程块上时菜单给「编辑 / 删除」，点空白格才是「新建日程」；
    // 日程块下面一定压着一个有效日期，所以先后顺序对新建那条分支没有影响
    CMonthScheduleItem *item = scheduleItemAt(event->pos());
    const QDate date = item != nullptr ? item->getDate() : dateAt(mapToScene(event->pos()));
    if (!date.isValid()) {
        QGraphicsView::contextMenuEvent(event);
        return;
    }

    popupMenu(event->globalPos(), date, item == nullptr ? DSchedule::Ptr() : item->getData());
    event->accept();
}

QDate CMonthGraphicsview::dateAt(const QPointF &scenePos) const
{
    for (CMonthDayItem *item : m_DayItem) {
        if (item->rect().contains(scenePos) && item->getDate().isValid()) {
            return item->getDate();
        }
    }
    return QDate();
}

CMonthScheduleItem *CMonthGraphicsview::scheduleItemAt(const QPoint &viewPos) const
{
    // 格子背景项、农历项都不是日程块，dynamic_cast 失败自然返回 nullptr
    return dynamic_cast<CMonthScheduleItem *>(itemAt(viewPos));
}

void CMonthGraphicsview::popupMenu(const QPoint &globalPos, const QDate &date,
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
        menu.addAction(tr("New Schedule"), this, [this, date] {
            emit signalCreateSchedule(QDateTime(date, QTime(0, 0)));
        });
    }

    menu.exec(globalPos);
}
