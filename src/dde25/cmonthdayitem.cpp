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
 * 移植自 dde-calendar（src/calendar-client/src/view/graphicsItem/cmonthdayitem.*）。
 * 差异：班/休图标改用本项目 qrc 中的资源路径；系统活动色走 DDE25::systemActiveColor()。
 */

#include "cmonthdayitem.h"

#include "constants.h"
#include "uitheme.h"

#include <QFontMetrics>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

bool CMonthDayItem::m_LunarVisible = false;

CMonthDayItem::CMonthDayItem(QGraphicsItem *parent)
    : CSceneBackgroundItem(CSceneBackgroundItem::OnMonthView, parent)
    , m_DayLunar("")
    , m_DayStatus(H_NONE)
{
    m_dayNumFont.setPixelSize(DDECalendar::FontSizeTwentyfour);
    m_dayNumFont.setWeight(QFont::Light);

    m_LunerFont.setPixelSize(DDECalendar::FontSizeTwelve);
    m_LunerFont.setWeight(QFont::Normal);
}

CMonthDayItem::~CMonthDayItem() = default;

void CMonthDayItem::setLunar(const QString &lunar)
{
    m_DayLunar = lunar;
}

void CMonthDayItem::setStatus(const CMonthDayItem::HolidayStatus &status)
{
    m_DayStatus = status;
}

void CMonthDayItem::setTheMe(int type)
{
    m_themetype = type;

    // 日期数字、今日、农历：全部从主题色派生
    m_dayNumColor = DDE25::textColor();
    m_dayNumCurrentColor = DDE25::onAccentColor();
    m_LunerColor = DDE25::textColor(DDE25::TextTertiary);
    m_fillColor = DDE25::panelColor();

    // 班/休两个标记只用主题色里的警示色与主色，不另引入红绿两种色相
    // （见 uitheme.h 的约束 1）
    m_banColor = DDE25::alertColor();
    m_banColor.setAlphaF(0.1);
    m_xiuColor = DDE25::accentColor();
    m_xiuColor.setAlphaF(0.1);

    m_BorderColor = DDE25::borderColor();
    update();
}

void CMonthDayItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    const int hh = 36;
    m_currentColor = getSystemActiveColor();
    painter->setRenderHints(QPainter::Antialiasing);

    // 绘制背景
    if (m_LunarVisible) {
        switch (m_DayStatus) {
        case H_WORK:
            painter->setBrush(QBrush(m_banColor));
            break;
        case H_REST:
            painter->setBrush(QBrush(m_xiuColor));
            break;
        default:
            painter->setBrush(QBrush(m_fillColor));
            break;
        }
    } else {
        painter->setBrush(QBrush(m_fillColor));
    }

    if (!m_IsCurrentMonth) {
        painter->setOpacity(0.4);
    }

    QPen pen;
    pen.setWidth(1);
    pen.setColor(m_BorderColor);
    painter->setPen(pen);
    painter->drawRect(this->rect());
    painter->save();

    // 绘制日期
    painter->setFont(m_dayNumFont);
    QRectF fillRect;
    if (m_LunarVisible) {
        fillRect.setRect(this->rect().x() + 3, this->rect().y() + 4, hh, hh);
    } else {
        fillRect.setRect(this->rect().x(), this->rect().y() + 4, this->rect().width(), hh);
    }

    // 今日高亮：参考 DDE 15，用一整块圆角主色底代替只圈住数字的小圆，
    // 日期与农历都换成底色上的文字色，远看也能一眼找到今天。
    // 色块只铺在日期/农历所在的头部（月视图的日程块从 y=31 开始），
    // 这样既醒目，又不会把当天的日程压在色块下面。
    const bool isToday = m_Date == QDate::currentDate();
    if (isToday) {
        painter->setOpacity(1);
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(m_currentColor));
        // 有农历时色块横贯整格，把日期和农历一起包进去；
        // 不显示农历时日期是居中的，色块也收成数字大小，免得成一条空横条。
        QRectF todayRect;
        if (m_LunarVisible) {
            todayRect.setRect(this->rect().x() + 4,
                              this->rect().y() + 3,
                              this->rect().width() - 8,
                              hh - 8);
        } else {
            const qreal blockWidth = hh - 8 + 16;
            todayRect.setRect(this->rect().x() + (this->rect().width() - blockWidth) / 2,
                              this->rect().y() + 3,
                              blockWidth,
                              hh - 8);
        }
        painter->drawRoundedRect(todayRect, DDE25::Metric::RadiusItem,
                                 DDE25::Metric::RadiusItem);
        painter->restore();
        painter->setPen(m_dayNumCurrentColor);
    } else {
        painter->setPen(m_dayNumColor);
    }

    fillRect.setY(fillRect.y() - 10);
    fillRect.setX(fillRect.x() - 1);
    painter->drawText(fillRect, Qt::AlignCenter, QString::number(m_Date.day()));
    painter->restore();

    // 绘制农历
    if (m_LunarVisible) {
        painter->save();
        if (isToday) {
            // 今日的色块是不透明的，农历也要跟着不透明，否则非当前月的今日
            // 会出现「实心色块 + 半透明农历」的错位感
            painter->setOpacity(1);
        }
        const QFontMetrics metrics(m_LunerFont);
        const int lunarWidth = metrics.horizontalAdvance(m_DayLunar);

        // 四个字的农历且有补班/休状态时换行显示
        const bool needWrap = (m_DayLunar.length() >= 4 && m_DayStatus != H_NONE);

        if (needWrap) {
            const QRectF iconRect(this->rect().x() + this->rect().width() - 55,
                                  this->rect().y() + 10, 12, 12);
            painter->setRenderHint(QPainter::Antialiasing);
            switch (m_DayStatus) {
            case H_WORK: {
                const QPixmap pixmap = QIcon(":/resources/icon/dde_calendar_ban_32px.svg").pixmap(QSize(12, 12));
                painter->drawPixmap(iconRect.toRect(), pixmap);
            } break;
            case H_REST: {
                const QPixmap pixmap = QIcon(":/resources/icon/dde_calendar_xiu.svg").pixmap(QSize(12, 12));
                painter->drawPixmap(iconRect.toRect(), pixmap);
            } break;
            default:
                break;
            }

            painter->setFont(m_LunerFont);
            painter->setPen(isToday ? m_dayNumCurrentColor : m_LunerColor);
            const QString firstLine = m_DayLunar.mid(0, 2);
            const QString secondLine = m_DayLunar.mid(2);

            painter->drawText(QRectF(this->rect().x() + this->rect().width() - 38,
                                     this->rect().y() + 6, 40, 18),
                              Qt::AlignLeft | Qt::AlignVCenter, firstLine);

            painter->drawText(QRectF(this->rect().x() + this->rect().width() - 56,
                                     this->rect().y() + 22, 58, 18),
                              Qt::AlignCenter, secondLine);
        } else {
            const qreal fillRectX = this->rect().width() - 12 - 3 - (58 + lunarWidth) / 2;
            const QSize iconSize = QSize(14, 14);
            const QRect iconRect(QPoint(this->rect().x() + fillRectX, this->rect().y() + 9), iconSize);

            if (fillRectX > hh) {
                painter->setRenderHint(QPainter::Antialiasing);
                switch (m_DayStatus) {
                case H_WORK: {
                    const QPixmap pixmap = QIcon(":/resources/icon/dde_calendar_ban_32px.svg").pixmap(iconSize);
                    painter->drawPixmap(iconRect, pixmap);
                } break;
                case H_REST: {
                    const QPixmap pixmap = QIcon(":/resources/icon/dde_calendar_xiu.svg").pixmap(iconSize);
                    painter->drawPixmap(iconRect, pixmap);
                } break;
                default:
                    break;
                }
            }

            painter->setFont(m_LunerFont);
            painter->setPen(isToday ? m_dayNumCurrentColor : m_LunerColor);
            painter->drawText(QRectF(this->rect().x() + this->rect().width() - 58,
                                     this->rect().y() + 6, 58, 18),
                              Qt::AlignCenter, m_DayLunar);
        }
        painter->restore();
    }

    // 如果有焦点则绘制焦点效果
    if (getItemFocus()) {
        const int offset = 1;
        const QRectF drawRect(rect().x() + offset, rect().y() + offset,
                              rect().width() - offset * 2, rect().height() - offset * 2);
        QPen framePen;
        framePen.setWidth(2);
        framePen.setColor(m_currentColor);
        painter->setPen(framePen);
        painter->setBrush(Qt::NoBrush);
        painter->setOpacity(1);
        const qreal radius = 16;
        const qreal diameter = radius * 2;

        QPainterPath path;
        path.moveTo(drawRect.x(), drawRect.y());
        // 左下角
        if (this->getBackgroundNum() == 35) {
            path.lineTo(drawRect.x(), drawRect.y() + drawRect.height() - radius);
            path.arcTo(QRectF(drawRect.x(), drawRect.y() + drawRect.height() - diameter, diameter, diameter), 180, 90);
        } else {
            path.lineTo(drawRect.x(), drawRect.y() + drawRect.height());
        }
        // 右下角
        if (this->getBackgroundNum() == 41) {
            path.lineTo(drawRect.x() + drawRect.width() - radius, drawRect.y() + drawRect.height());
            path.arcTo(QRectF(drawRect.x() + drawRect.width() - diameter, drawRect.y() + drawRect.height() - diameter, diameter, diameter), 270, 90);
        } else {
            path.lineTo(drawRect.x() + drawRect.width(), drawRect.y() + drawRect.height());
        }
        path.lineTo(drawRect.x() + drawRect.width(), drawRect.y());
        path.lineTo(drawRect.x(), drawRect.y());
        painter->drawPath(path);
    }
}
