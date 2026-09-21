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

    if (type == 0 || type == 1) {
        m_dayNumColor = QColor(0, 0, 0, 204);
        m_dayNumCurrentColor = "#FFFFFF";

        m_LunerColor = QColor(0, 0, 0, 102);

        m_fillColor = Qt::white;
        m_banColor = "#FF7171";
        m_banColor.setAlphaF(0.1);
        m_xiuColor = "#ADFF71";
        m_xiuColor.setAlphaF(0.1);

        m_BorderColor = "#000000";
        m_BorderColor.setAlphaF(0.05);
    } else if (type == 2) {
        m_dayNumColor = QColor(255, 255, 255, 204);
        m_dayNumCurrentColor = "#B8D3FF";

        m_LunerColor = QColor(255, 255, 255, 102);

        m_fillColor = "#000000";
        m_fillColor.setAlphaF(0.05);
        m_banColor = "#FF7171";
        m_banColor.setAlphaF(0.1);
        m_xiuColor = "#ADFF71";
        m_xiuColor.setAlphaF(0.1);

        m_BorderColor = QColor(255, 255, 255, 13);
    }
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

    // 如果为当前时间
    if (m_Date == QDate::currentDate()) {
        painter->setOpacity(1);
        QFont tFont = m_dayNumFont;
        tFont.setPixelSize(DDECalendar::FontSizeTwenty);
        painter->setFont(tFont);
        painter->setPen(m_dayNumCurrentColor);
        painter->save();
        painter->setBrush(QBrush(m_currentColor));
        painter->setPen(Qt::NoPen);
        if (m_LunarVisible) {
            painter->drawEllipse(QRectF(this->rect().x() + 6, this->rect().y() + 4, hh - 8, hh - 8));
        } else {
            painter->drawEllipse(QRectF((this->rect().width() - hh + 8) / 2 + this->rect().x(),
                                        this->rect().y() + 4, hh - 8, hh - 8));
        }
        painter->restore();
    } else {
        painter->setPen(m_dayNumColor);
    }

    fillRect.setY(fillRect.y() - 10);
    fillRect.setX(fillRect.x() - 1);
    painter->drawText(fillRect, Qt::AlignCenter, QString::number(m_Date.day()));
    painter->restore();

    // 绘制农历
    if (m_LunarVisible) {
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
            painter->setPen(m_LunerColor);
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
            painter->setPen(m_LunerColor);
            painter->drawText(QRectF(this->rect().x() + this->rect().width() - 58,
                                     this->rect().y() + 6, 58, 18),
                              Qt::AlignCenter, m_DayLunar);
        }
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
