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
 * 移植自 dde-calendar（src/calendar-client/src/dialog/schedulectrldlg.*）。
 * 交互与对外接口不变，替换的只有底层依赖：
 *
 * 1. DTK2Widget 没有 DFontSizeManager，参考实现绑的 T6 按本项目既有做法
 *    （见 scheduleitem.cpp 的实测注释）折算成 12px，直接 setPixelSize；
 * 2. DPalette::WindowText / DPalette::Window 换成 QPalette 的同名角色；
 * 3. 主题切换原来连 DGuiApplicationHelper::themeTypeChanged，这里在 changeEvent
 *    里监听调色板变化后重新 setTheMe(DDE25::themeType())；
 * 4. 对话框图标原来取 CDynamicIcon（随日期变化），这里用应用自己的图标。
 */

#ifndef CSCHEDULECTRLDLG_H
#define CSCHEDULECTRLDLG_H

#include "dcalendarddialog.h"

//dtk2 的 <DFrame> 包装头和 dframe.h 共用 DFRAME_H 宏，先定义再 include，
//dframe.h 的主体整个被跳过，include <DFrame> 等于没做（daymonthview.h 里有同样的记录），
//这里直接 include 小写的实体头
#include <dframe.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QObject>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

class CScheduleCtrlDlg : public DCalendarDDialog
{
    Q_OBJECT
public:
    explicit CScheduleCtrlDlg(QWidget *parent = nullptr);
    QAbstractButton *addPushButton(QString btName, bool type = false);
    QAbstractButton *addsuggestButton(QString btName, bool type = false);
    QAbstractButton *addWaringButton(QString btName, bool type = false);
    void setText(QString str);
    void setInformativeText(QString str);
    int clickButton();
private:
    void initUI();
    void initConnection();
    /**
     * @brief setTheMe  根据主题type设置颜色
     * @param type  主题type
     */
    void setTheMe(const int type);
    /**
     * @brief setPaletteTextColor   设置调色板颜色
     * @param widget    需要设置的widget
     * @param textColor     显示颜色
     */
    void setPaletteTextColor(QWidget *widget, QColor textColor);
protected:
    void changeEvent(QEvent *event) override;
private slots:
    void buttonJudge(int id);
private:
    QLabel                           *m_firstLabel = nullptr;
    QLabel                           *m_seconLabel = nullptr;
    int                              m_id = -1;
    QVBoxLayout *m_mainBoxLayout = nullptr;
    DFrame *gwi = nullptr;
    QFont labelF;
    QFont labelT;
    QVector<QString> str_btName;
};

#endif // CSCHEDULECTRLDLG_H
