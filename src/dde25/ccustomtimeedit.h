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
 * 移植自 dde-calendar（src/calendar-client/src/customWidget/ccustomtimeedit.*）。
 * 只用到 Qt，除去掉日志宏外没有任何改动。
 */

#ifndef CCUSTOMTIMEEDIT_H
#define CCUSTOMTIMEEDIT_H

#include <QDateTimeEdit>

/**
 * @brief The CCustomTimeEdit class
 */
class CCustomTimeEdit : public QTimeEdit
{
    Q_OBJECT
public:
    explicit CCustomTimeEdit(QWidget *parent = nullptr);
    //获取编辑框
    QLineEdit *getLineEdit();
protected:
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
signals:
    void signalUpdateFocus(bool showFocus);
};

#endif // CCUSTOMTIMEEDIT_H
