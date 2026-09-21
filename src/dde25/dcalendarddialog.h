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
 * 移植自 dde-calendar（src/calendar-client/src/dialog/dcalendarddialog.*）。
 *
 * 差异：
 * 1. 本项目没有 CalendarManager 这个全局时间/日期制式管理器，构造时用默认值
 *    （24 小时制 + yyyy-MM-dd），setTimeFormat()/setDateFormat() 保留原样，
 *    需要时由调用方设置；
 * 2. 去掉平板模式的 TabletConfig 判断，mouseMoveEvent 直接走 DDialog；
 * 3. 去掉 DTK_VERSION 分支（DTK2Widget 一定是新版 DDialog）。
 */

#ifndef DCALENDARDDIALOG_H
#define DCALENDARDDIALOG_H

#include <DDialog>

DWIDGET_USE_NAMESPACE

class DCalendarDDialog : public DDialog
{
    Q_OBJECT
public:
    explicit DCalendarDDialog(QWidget *parent = nullptr);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *o, QEvent *e) override;
    virtual void updateDateTimeFormat();
signals:

public slots:
    void setTimeFormat(int value);
    void setDateFormat(int value);
protected:
    QString m_timeFormat = "hh:mm";
    QString m_dateFormat = "yyyy-MM-dd";
};

#endif // DCALENDARDDIALOG_H
