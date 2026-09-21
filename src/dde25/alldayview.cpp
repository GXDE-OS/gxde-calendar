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
 */

#include "alldayview.h"

#include "calldayscheduleitem.h"
#include "constants.h"
#include "dde25common.h"
#include "schedulecoormanage.h"
#include "schedulelayout.h"

#include <QEvent>
#include <QFontMetrics>
#include <QGraphicsScene>
#include <QPainter>
#include <QResizeEvent>
#include <QtGlobal>

CAllDayView::CAllDayView(QWidget *parent, ViewPosition viewPos)
    : CWeekGraphicsView(parent, viewPos)
{
    // 全天条没有整点网格，不该画横线
    m_LRFlag = false;
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    updateItemHeightByFontSize();
    // 基类构造函数里的 setTheMe 不走虚函数，这里补一次把自己的成员也刷上
    setTheMe(DDE25::themeType());
}

CAllDayView::~CAllDayView() = default;

void CAllDayView::setAllDayInfo(const DSchedule::List &info)
{
    m_allDayInfo = info;
    updateInfo();
}

void CAllDayView::setRange(int w, int h, QDate begindate, QDate enddate, int rightmagin)
{
    // 全天条的高度由行数算出来，跟时间网格的 24 小时高度无关，h 不参与
    Q_UNUSED(h)
    if (w <= 0) {
        return;
    }
    m_width = w;
    m_beginDate = begindate;
    m_endDate = enddate;
    m_rightmagin = rightmagin;
    setBackgroundDate();
    updateInfo();
}

void CAllDayView::setRange(QDate begin, QDate end)
{
    m_beginDate = begin;
    m_endDate = end;
    setBackgroundDate();
    updateInfo();
}

DSchedule::Ptr CAllDayView::scheduleAt(const QPoint &viewPos) const
{
    CAllDayScheduleItem *item = dynamic_cast<CAllDayScheduleItem *>(itemAt(viewPos));
    return item == nullptr ? DSchedule::Ptr() : item->getData();
}

QDateTime CAllDayView::scheduleDateTimeAt(const QPoint &viewPos) const
{
    // updateDateShow() 里给 m_coorManage 设过范围，所以 getsDate 是有效的；
    // 还没排过版（日期无效）时直接放弃
    if (!m_beginDate.isValid() || !m_endDate.isValid()) {
        return QDateTime();
    }
    const QDate date = m_coorManage->getsDate(mapToScene(viewPos));
    if (!date.isValid()) {
        return QDateTime();
    }
    return QDateTime(date, QTime(0, 0));
}

void CAllDayView::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        m_dividingLineColor = QColor(0, 0, 0, 13);
    } else {
        m_dividingLineColor = QColor(255, 255, 255, 10);
    }
    CWeekGraphicsView::setTheMe(type);
    update();
}

void CAllDayView::clearSchedule()
{
    clearItems();
    m_allDayInfo.clear();
    updateInfo();
}

void CAllDayView::clearItems()
{
    for (CAllDayScheduleItem *item : m_baseShowItem) {
        if (m_Scene) {
            m_Scene->removeItem(item);
        }
        delete item;
    }
    m_baseShowItem.clear();
}

/**
 * @brief CAllDayView::updateInfo   重排全天日程条
 *
 * 对应参考实现的 CAllDayEventWeekView::upDateInfoShow：先把全天日程按行打包，
 * 再由行数定这一条的高度。参考实现是把「行 × 天」填一张矩阵来做打包，
 * 这里换成等价的贪心打包（DDE25::packAllDayRows），结果一致但不用建矩阵。
 */
void CAllDayView::updateInfo()
{
    clearItems();

    if (!m_beginDate.isValid() || !m_endDate.isValid() || m_allDayInfo.isEmpty()) {
        m_vlistData.clear();
    } else {
        m_vlistData = DDE25::packAllDayRows(m_allDayInfo, m_beginDate, m_endDate);
    }

    // 行数定高度：不给或 1 行只有一条 29px 的窄条，2~5 行按行高递增，6 行及以上封顶
    int topMargin;
    if (m_vlistData.size() < 2) {
        topMargin = 32;
    } else if (m_vlistData.size() < 6) {
        topMargin = 31 + (m_vlistData.size() - 1) * (m_itemHeight + 1);
    } else {
        topMargin = 123;
    }
    const int bandHeight = topMargin - 3;
    if (height() != bandHeight) {
        setFixedHeight(bandHeight);
    }

    updateDateShow();
    update();

    // 外层靠这个信号挪「ALL DAY」标签和分隔线
    emit signalUpdatePaint(bandHeight);
}

/**
 * @brief CAllDayView::updateDateShow   按行数定场景高度并铺开日程块
 */
void CAllDayView::updateDateShow()
{
    const qreal itemsHeight = (m_itemHeight + 2) * m_vlistData.size();
    qreal sceneHeight = itemsHeight < 32 ? 29 : itemsHeight + 6;
    // 场景不能比控件矮，否则各列背景的分隔线画不到底
    sceneHeight = qMax(sceneHeight, qreal(qMax(viewport()->height(), height())));

    setSceneRect(0, 0, m_width, sceneHeight);
    m_coorManage->setRange(m_width, qRound(sceneHeight), m_beginDate, m_endDate, m_rightmagin);

    for (int i = 0; i < m_vlistData.size(); ++i) {
        createItemWidget(i);
    }
}

void CAllDayView::createItemWidget(int index)
{
    // 宽度还没定下来（首次布局前 setRange(QDate,QDate) 会先跑一遍）就先不建块，
    // 等 resize 时带上真实宽度重新进来一次
    if (m_width <= 0) {
        return;
    }
    for (const DSchedule::Ptr &info : m_vlistData.at(index)) {
        if (info.isNull()) {
            continue;
        }
        // 横向位置和宽度由坐标管理器按起止日期裁出来，纵向按行号叠
        QRectF drawrect = m_coorManage->getAllDayDrawRegion(info->dtStart().date(), info->dtEnd().date());
        drawrect.setY(2 + (m_itemHeight + 1) * index);
        drawrect.setHeight(m_itemHeight + 1);

        CAllDayScheduleItem *item = new CAllDayScheduleItem(drawrect);
        item->setData(info);
        m_Scene->addItem(item);
        m_baseShowItem.append(item);
    }
}

void CAllDayView::updateItemHeightByFontSize()
{
    QFont font;
    font.setPixelSize(DDECalendar::FontSizeTen);
    const QFontMetrics fm(font);
    m_itemHeight = fm.height() + 1;
}

void CAllDayView::paintEvent(QPaintEvent *event)
{
    // 全天条只画底部一条与时间网格的分隔线，整点横线交给时间网格自己画
    QGraphicsView::paintEvent(event);

    QPainter painter(viewport());
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_dividingLineColor);
    painter.drawRect(QRect(0, viewport()->height() - 1,
                           viewport()->width() - m_rightmagin, 1));
}

void CAllDayView::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        updateItemHeightByFontSize();
        updateInfo();
    }
    CWeekGraphicsView::changeEvent(event);
}
