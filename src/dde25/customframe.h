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
 * 移植自 dde-calendar（src/calendar-client/src/customWidget/customframe.*）。
 */

#ifndef CUSTOMFRAME_H
#define CUSTOMFRAME_H

#include <QColor>
#include <QFont>
#include <QFrame>
#include <QPainterPath>
#include <QRectF>

/**
 * @brief 可指定哪些角是圆角的填充框，dde-calendar 用它拼出头部/表头的圆角造型。
 */
class CustomFrame : public QFrame
{
    Q_OBJECT
public:
    explicit CustomFrame(QWidget *parent = nullptr);

    void setBColor(QColor normalC);
    void setRoundState(bool lstate, bool tstate, bool rstate, bool bstate);
    void setTextStr(const QFont &font, const QColor &tc, const QString &strc,
                    int flag = Qt::AlignCenter);
    void setTextStr(const QString &strc);
    void setTextColor(QColor tc);
    void setTextFont(const QFont &font);
    void setTextAlign(int flag = Qt::AlignCenter);
    void setRadius(int radius = 8);
    void setboreder(int framew = 0);
    // 1px 圆角描边色，alpha 为 0 时不画（默认值），既有用法都不受影响。
    // 加它是为了周视图周数条的外框（weekwindow 的 m_todayframe）能对齐月视图月份条
    // 由 CMonthDayView::paintEvent 画出来的那个圆角外框——参考实现的 CustomFrame
    // 本来就没有描边，只有 setBColor 的填充。
    void setBorderColor(QColor borderC);

    QString getTextStr() { return m_text; }

    void setFixedSize(int w, int h);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    // 按四个角各自的圆角开关拼出边框路径
    QPainterPath roundedPath(const QRectF &rect) const;

    QColor m_bnormalColor = "#FFFFFF";
    QColor m_tnormalColor = "#000000";
    QColor m_borderColor = QColor(0, 0, 0, 0);
    QFont m_font;
    bool m_bflag = false;
    bool m_fixsizeflag = false;
    int m_textflag = Qt::AlignCenter; // 对齐方式
    QString m_text;
    int m_radius = 8;
    int m_borderframew = 0;
    bool m_lstate = false;
    bool m_tstate = false;
    bool m_rstate = false;
    bool m_bstate = false;
};

#endif // CUSTOMFRAME_H
