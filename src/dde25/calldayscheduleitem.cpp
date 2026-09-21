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
 * 移植自 dde-calendar（src/calendar-client/src/view/graphicsItem/calldayscheduleitem.*）。
 */

#include "calldayscheduleitem.h"

#include "constants.h"
#include "dde25common.h"
#include "schedulecolors.h"

#include <QFontMetrics>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QPen>

// 参考实现用 DFontSizeManager 的 T8 取字号（默认 dtk6 字号下实测 10px），
// 与 CScheduleItem 里时间那一行的取字号方式保持一致。
static const int AllDayItemFontSize = DDECalendar::FontSizeTen;

CAllDayScheduleItem::CAllDayScheduleItem(QRectF rect, QGraphicsItem *parent)
    : CFocusItem(parent)
{
    setRect(rect);
    setItemType(CITEM);
    setAcceptHoverEvents(true);
}

CAllDayScheduleItem::~CAllDayScheduleItem() = default;

void CAllDayScheduleItem::setData(const DSchedule::Ptr &info)
{
    m_vScheduleInfo = info;
    update();
}

/**
 * @brief CAllDayScheduleItem::hasSelectSchedule      是否含有选中日程
 * @param info
 * @return
 */
bool CAllDayScheduleItem::hasSelectSchedule(const DSchedule::Ptr &info)
{
    return info == m_vScheduleInfo;
}

void CAllDayScheduleItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_hover = true;
    update();
    CFocusItem::hoverEnterEvent(event);
}

void CAllDayScheduleItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_hover = false;
    update();
    CFocusItem::hoverLeaveEvent(event);
}

void CAllDayScheduleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                                QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (m_vScheduleInfo.isNull()) {
        return;
    }

    paintBackground(painter, rect());
}

/**
 * @brief CAllDayScheduleItem::paintBackground        绘制item显示效果
 * @param painter
 * @param rect
 *
 * 参考实现里这是 DragInfoItem 的纯虚函数重载，入口还带着一个 isPixMap 参数
 * （画拖拽动画位图用），本阶段没有拖拽，改成不带该参数的私有辅助函数。
 */
void CAllDayScheduleItem::paintBackground(QPainter *painter, const QRectF &rect)
{
    // 按日程类型取色。参考实现走 CScheduleDataManage 单例，这里用无状态的自由函数。
    DDE25::CSchedulesColor gdColor = DDE25::scheduleColorByType(m_vScheduleInfo->scheduleTypeID());
    QColor textcolor = DDE25::scheduleTextColor();

    // 参考实现拿 DragInfoItem 的静态拖拽信息判断「按下」「选中」两种状态，本项目没有
    // 拖拽交互，选中态改用 CFocusItem 的键盘 focus 表示（与 CScheduleItem 一致）。
    const bool selectflag = getItemFocus();

    QColor brushColor = gdColor.normalColor;
    if (m_hover) {
        brushColor = gdColor.hoverColor;
    } else if (selectflag) {
        brushColor = gdColor.pressColor;
        textcolor.setAlphaF(0.4);
    }

    // 底部留 2px 不画，让相邻两行之间透出背景
    const QRectF fillRect(rect.x(), rect.y(), rect.width(), rect.height() - 2);
    const qreal radius = rect.height() / 3.0;

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(brushColor);
    if (selectflag) {
        // 键盘 focus 用系统活动色描边
        QPen framePen;
        framePen.setWidth(2);
        framePen.setColor(getSystemActiveColor());
        painter->setPen(framePen);
    } else {
        painter->setPen(Qt::NoPen);
    }
    painter->drawRoundedRect(fillRect, radius, radius);

    // 左侧 2px 竖线，颜色跟随日程所属日历
    painter->save();
    QColor sourceColor = QColor(m_vScheduleInfo->accountColor());
    if (!sourceColor.isValid()) {
        sourceColor = gdColor.orginalColor;
    }
    QPen sourcePen(sourceColor);
    sourcePen.setWidth(2);
    painter->setPen(sourcePen);
    painter->drawLine(QPointF(fillRect.left(), fillRect.top() + 1),
                      QPointF(fillRect.left(), fillRect.bottom() - 1));
    painter->restore();

    // 标题只画一行，左侧偏移 13、右侧偏移 8
    QFont font;
    font.setWeight(QFont::Normal);
    font.setPixelSize(AllDayItemFontSize);
    painter->setFont(font);
    painter->setPen(textcolor);

    const QFontMetrics fm = painter->fontMetrics();
    QString str = m_vScheduleInfo->summary();
    str.replace("\n", "");
    const qreal textWidth = fillRect.width() - 13 - 8;

    QString tStr;
    qreal showWidth = textWidth;
    if (fm.horizontalAdvance(str) > showWidth) {
        // 放不下就逐字加、超了退回一个字，末尾补 "..."
        showWidth -= fm.horizontalAdvance("...");
        for (int i = 0; i < str.size(); i++) {
            tStr.append(str.at(i));
            if (fm.horizontalAdvance(tStr) > showWidth) {
                tStr.chop(1);
                break;
            }
        }
        if (tStr != str) {
            tStr += "...";
        }
    } else {
        tStr = str;
    }

    painter->drawText(QRectF(fillRect.left() + 13, fillRect.y() - 1, textWidth, fillRect.height() - 1),
                      Qt::AlignLeft | Qt::AlignVCenter, tStr);

    if (m_hover && !selectflag) {
        // 悬停时沿块边缘描一圈 8% 透明色
        const QRectF tRect(fillRect.x() + 0.5, fillRect.y() + 0.5,
                           fillRect.width() - 1, fillRect.height() - 1);
        painter->save();
        QPen pen;
        QColor selcolor = (DDE25::themeType() == 2) ? QColor("#FFFFFF") : QColor("#000000");
        selcolor.setAlphaF(0.08);
        pen.setColor(selcolor);
        pen.setWidthF(1);
        pen.setStyle(Qt::SolidLine);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(pen);
        painter->drawRoundedRect(tRect, radius, radius);
        painter->restore();
    }
    if (selectflag) {
        // 选中时整块再叠一层 5% 透明色。参考实现用的是固定黑色，
        // 深色主题下叠黑反而发闷，这里跟随主题取色（与 CScheduleItem 一致）
        QColor selcolor = (DDE25::themeType() == 2) ? QColor("#FFFFFF") : QColor("#000000");
        selcolor.setAlphaF(0.05);
        painter->setBrush(selcolor);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(fillRect, radius, radius);
    }
}
