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
 * 移植自 dde-calendar（src/calendar-client/src/view/graphicsItem/scheduleitem.*）。
 */

#include "scheduleitem.h"

#include "constants.h"
#include "dde25common.h"
#include "schedulecolors.h"

#include <QFontMetrics>
#include <QGraphicsSceneHoverEvent>
#include <QMarginsF>
#include <QPainter>
#include <QPen>

// 参考实现用 DFontSizeManager::T8（时间）和 T6（标题）取字号。DFontSizeManager 会
// 跟随系统字号缩放，而本项目其它视图一律用 DDECalendar::FontSize* 固定像素；
// dtk6 默认字号下实测 T8 = 10px、T6 = 12px，这里就取这两个值。
static const int ItemTimeFontSize = DDECalendar::FontSizeTen;
static const int ItemTitleFontSize = DDECalendar::FontSizeTwelve;

CScheduleItem::CScheduleItem(QRectF rect, QGraphicsItem *parent, int type)
    : CFocusItem(parent)
    , m_type(type)
{
    setRect(rect);
    setItemType(CITEM);
    setAcceptHoverEvents(true);
}

CScheduleItem::~CScheduleItem() = default;

/**
 * @brief CScheduleItem::setData        设置显示数据
 * @param info
 * @param date
 * @param totalNum
 */
void CScheduleItem::setData(const DSchedule::Ptr &info, QDate date, int totalNum)
{
    m_vScheduleInfo = info;
    m_totalNum = totalNum;
    setDate(date);
    update();
}

/**
 * @brief CScheduleItem::hasSelectSchedule      是否含有选中日程
 * @param info
 * @return
 */
bool CScheduleItem::hasSelectSchedule(const DSchedule::Ptr &info)
{
    return info == m_vScheduleInfo;
}

/**
 * @brief CScheduleItem::splitText      根据字体大小,宽度和高度将标题切换为多行
 * @param font
 * @param w
 * @param h
 * @param str
 * @param listStr
 * @param fontM
 */
void CScheduleItem::splitText(QFont font, int w, int h, QString str, QStringList &listStr, QFontMetrics &fontM)
{
    if (str.isEmpty()) {
        return;
    }
    QFontMetrics fontMetrics(font);
    int heightT = fontM.height();
    QString tStr;
    QStringList tListStr;

    for (int i = 0; i < str.count(); i++) {
        tStr.append(str.at(i));
        int widthT = fontMetrics.horizontalAdvance(tStr) + 5;

        if (widthT >= w) {
            tStr.chop(1);
            if (tStr.isEmpty())
                break;
            tListStr.append(tStr);
            tStr.clear();
            i--;
        }
    }
    tListStr.append(tStr);

    if (w < 30) {
        // 块太窄时一行都放不下，只保留首字加省略号
        QFontMetrics f_st(font);
        if (h < 23) {
            tListStr.append("");
        } else {
            if (tListStr.isEmpty()) {
                listStr.append("");
            } else {
                QString c = str.at(0);
                QString str = c + "...";
                QFontMetrics fm(font);
                while (f_st.horizontalAdvance(str) > w && f_st.horizontalAdvance(str) > 24) {
                    str.chop(1);
                }
                listStr.append(str);
            }
        }
    } else {
        for (int i = 0; i < tListStr.count(); i++) {
            if ((i + 1) * heightT <= h - 1) {
                listStr.append(tListStr.at(i));
            } else {
                if (i == 0) {
                    break;
                } else {
                    QString s;
                    // 放不下的那一行用省略号收尾
                    if (i == tListStr.count())
                        s = fontM.elidedText(tListStr.at(i - 1), Qt::ElideRight, w);
                    else {
                        s = fontM.elidedText(tListStr.at(i - 1) + "...", Qt::ElideRight, w);
                    }
                    listStr.removeAt(i - 1);
                    listStr.append(s);
                    break;
                }
            }
        }
    }
}

void CScheduleItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_hover = true;
    update();
    CFocusItem::hoverEnterEvent(event);
}

void CScheduleItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_hover = false;
    update();
    CFocusItem::hoverLeaveEvent(event);
}

void CScheduleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
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
 * @brief CScheduleItem::paintBackground        绘制item显示效果
 * @param painter
 * @param rect
 *
 * 参考实现里这是 DragInfoItem 的纯虚函数重载，入口还带着一个 isPixMap 参数
 * （画拖拽动画位图用），本阶段没有拖拽，改成不带该参数的私有辅助函数。
 */
void CScheduleItem::paintBackground(QPainter *painter, const QRectF &rect)
{
    // 按日程类型取色。参考实现走 CScheduleDataManage 单例，这里用无状态的自由函数。
    DDE25::CSchedulesColor gdColor = DDE25::scheduleColorByType(m_vScheduleInfo->scheduleTypeID());
    QColor textPenColor = DDE25::scheduleTextColor();

    // 参考实现拿 DragInfoItem 的静态拖拽信息判断「选中」「高亮」两种状态，本项目没有
    // 拖拽交互，选中态改用 CFocusItem 的键盘 focus 表示（与月视图的移植保持一致）。
    const bool selectflag = getItemFocus();

    QColor bColor = gdColor.normalColor;
    QFontMetrics fm = painter->fontMetrics();
    int h = fm.height();

    if (m_hover) {
        bColor = gdColor.hoverColor;
    } else if (selectflag) {
        bColor = gdColor.pressColor;
    }
    painter->setBrush(bColor);
    painter->setPen(Qt::NoPen);
    painter->drawRect(rect);

    if (m_hover && !selectflag) {
        // 悬停时沿块边缘描一圈 8% 透明色
        painter->save();
        QRectF tRect = QRectF(rect.x() + 0.5, rect.y() + 0.5, rect.width() - 1, rect.height() - 1);
        QPen tPen;
        QColor cc = (DDE25::themeType() == 2) ? QColor("#FFFFFF") : QColor("#000000");
        cc.setAlphaF(0.08);
        tPen.setColor(cc);
        tPen.setWidthF(1);
        tPen.setStyle(Qt::SolidLine);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(tPen);
        painter->drawRect(tRect);
        painter->restore();
    }
    if (selectflag) {
        // 选中态在浅色主题下把文字和类型色压到 40% 透明，深色主题下压到 60%
        if (DDE25::themeType() == 0 || DDE25::themeType() == 1) {
            textPenColor.setAlphaF(0.4);
            gdColor.orginalColor.setAlphaF(0.4);
        } else if (DDE25::themeType() == 2) {
            textPenColor.setAlphaF(0.6);
            gdColor.orginalColor.setAlphaF(0.6);
        }
    }

    painter->save();
    QColor sourceColor = QColor(m_vScheduleInfo->accountColor());
    if (!sourceColor.isValid()) {
        sourceColor = gdColor.orginalColor;
    }
    QPen pen(sourceColor);
    pen.setWidth(2);
    painter->setPen(pen);
    //左侧绘制竖线
    QPointF top(rect.topLeft().x(), rect.topLeft().y() + 1);
    QPointF bottom(rect.bottomLeft().x(), rect.bottomLeft().y() - 1);
    painter->drawLine(top, bottom);
    painter->restore();

    // 块内左右留白。一格里有多个日程时挤一挤
    int tMargin = 10;
    if (m_totalNum > 1)
        tMargin = 5;

    if (m_type == 0) {
        int timeTextHight = 0;
        QFont font;
        font.setWeight(QFont::Normal);
        font.setPixelSize(ItemTimeFontSize);

        //绘制日程起始时间，只有起始时间在本块对应的日期上才画
        if (m_vScheduleInfo->dtStart().date() == getDate()) {
            painter->save();
            painter->setFont(font);
            painter->setPen(gdColor.orginalColor);

            // 参考实现这里还会按时间的 12/24 小时制式加上 "AP " 前缀，
            // 本项目没有该设置项，统一用 24 小时制
            QTime stime = m_vScheduleInfo->dtStart().toLocalTime().time();
            QString str = stime.toString(m_timeFormat);
            QFontMetrics fontMetrics(font);
            // 参考实现这里减的是拖拽动画的水平偏移量 m_offset，本阶段恒为 0，直接省掉
            qreal drawTextWidth = rect.width();
            QRectF timeRect(rect.topLeft().x() + tMargin, rect.topLeft().y() + 3,
                            drawTextWidth - 5, h);

            if (fm.horizontalAdvance(str) > drawTextWidth - 5) {
                // 时间都放不下（块极窄）时用省略号截断
                painter->drawText(timeRect, Qt::AlignLeft,
                                  fontMetrics.elidedText(str, Qt::ElideRight,
                                                         qRound(drawTextWidth - 5)));
            } else {
                painter->drawText(timeRect, Qt::AlignLeft, str);
            }
            painter->restore();
        } else {
            // 不是起始当天（跨天日程的续块），时间行空出来给标题用
            timeTextHight = -20;
        }
        painter->save();

        //绘制日程标题
        font.setPixelSize(ItemTitleFontSize);
        font.setLetterSpacing(QFont::PercentageSpacing, 105);
        painter->setFont(font);
        painter->setPen(textPenColor);
        QStringList liststr;
        QRect textRect = rect.toRect();
        splitText(font, textRect.width() - tMargin - 8, textRect.height() - 20,
                  m_vScheduleInfo->summary(), liststr, fm);

        for (int i = 0; i < liststr.count(); i++) {
            if ((20 + timeTextHight + (i + 1) * (h - 3)) > rect.height()) {
                // 参考实现这里直接 return：既漏画了后面的选中底色和 focus 边框，又留下
                // 一个未配对的 painter->save()。改成 break，只停止继续画标题。
                break;
            }
            painter->drawText(
                QRect(textRect.topLeft().x() + tMargin,
                      textRect.topLeft().y() + 20 + timeTextHight + i * (h - 3),
                      textRect.width() - 2,
                      h),
                Qt::AlignLeft, liststr.at(i));
        }
        painter->restore();
    } else {
        // m_type 非 0：调用方克隆一份日程当占位块（summary 置 "1"、类型置 "other"），
        // 表示这一格/这一时段还有排不下的日程，只画一个省略号
        painter->save();
        QFont font;
        font.setWeight(QFont::Normal);
        font.setPixelSize(ItemTimeFontSize);
        painter->setFont(font);
        painter->setPen(textPenColor);
        painter->drawText(rect, Qt::AlignCenter | Qt::AlignVCenter, "...");
        painter->restore();
    }
    if (selectflag) {
        // 选中时整块再叠一层 5% 透明色。参考实现用的是固定黑色，
        // 深色主题下叠黑反而发闷，这里跟随主题取色
        QColor selcolor = (DDE25::themeType() == 2) ? QColor("#FFFFFF") : QColor("#000000");
        selcolor.setAlphaF(0.05);
        painter->setBrush(selcolor);
        painter->setPen(Qt::NoPen);
        painter->drawRect(rect);
    }
    if (selectflag) {
        // 键盘 focus 的描边
        QRectF drawRect = rect.marginsRemoved(QMarginsF(1, 1, 1, 1));
        painter->setBrush(Qt::NoBrush);
        QPen framePen;
        framePen.setWidth(2);
        framePen.setColor(getSystemActiveColor());
        painter->setPen(framePen);
        painter->drawRect(drawRect);
    }
}
