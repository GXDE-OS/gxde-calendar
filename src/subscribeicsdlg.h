/*
 * Copyright (C) 2026 CharOfString <root@charofstring.cc>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ----------------------------------------------------------------------------
 *
 * 参考实现（dde-calendar 6.6.0）没有「订阅在线 ICS」这个功能，只有 CalDAV 账户，
 * 所以这个弹窗没有对应的移植源。外观沿用本项目已移植的 DCalendarDDialog：同样的
 * 标题栏、按钮排布，主题颜色跟着 DDE25::themeType() 走。
 */

#ifndef SUBSCRIBEICSDLG_H
#define SUBSCRIBEICSDLG_H

#include "dde25/dcalendarddialog.h"
#include "schedule/calendarservice.h"

//DFrame / DLineEdit 不能只写前置声明：DWIDGET_USE_NAMESPACE 会把 Dtk::Widget 下的
//同名类引进全局，跟前置声明的 ::DFrame 撞成二义（同 colorpickerwidget.h 的处理）。
//<DFrame> 包装头和 dframe.h 共用 DFRAME_H 宏，include 等于没做，直接取实体头。
#include <DLineEdit>
#include <dframe.h>

#include <QString>

class ColorSeletorWidget;
class QAbstractButton;
class QComboBox;
class QLabel;

/**
 * @brief 在线日历订阅的表单：地址 + 名称 + 刷新间隔 + 颜色。
 *
 * 新增和编辑共用：只负责收集输入，不碰数据层。调用方 exec() 拿到 Accepted 之后取
 * url()/displayName()/refreshIntervalMin()/colorCode()，新增时自己去调
 * CalendarService::subscribeIcs()，编辑时先 setEditData() 预填再调
 * CalendarService::updateIcsSubscription()。
 */
class CSubscribeIcsDlg : public DCalendarDDialog
{
    Q_OBJECT
public:
    explicit CSubscribeIcsDlg(QWidget *parent = nullptr);

    /**
     * @brief setEditData    按已有订阅预填表单，切换到「编辑」模式
     *
     * 改标题和按钮文字，让用户知道现在改的是哪个订阅（列表里点进来的那个）。
     */
    void setEditData(const CalendarService::IcsSubscriptionInfo &info);

    //下面四个在 exec() 返回 Accepted 之后取
    QString url() const;
    QString displayName() const;
    int refreshIntervalMin() const;
    QString colorCode() const;

protected:
    void changeEvent(QEvent *event) override;

private:
    void initUI();
    void initConnection();
    /**
     * @brief setTheMe  根据主题type设置标签颜色
     * @param type  主题type（见 DDE25::themeType()）
     */
    void setTheMe(const int type);
    //地址不是 http(s) 链接时把「订阅」按钮禁掉
    void updateAcceptState();

    DFrame *m_gwi = nullptr;
    QLabel *m_urlLabel = nullptr;
    QLabel *m_nameLabel = nullptr;
    QLabel *m_intervalLabel = nullptr;
    QLabel *m_colorLabel = nullptr;
    DLineEdit *m_urlEdit = nullptr;
    DLineEdit *m_nameEdit = nullptr;
    QComboBox *m_intervalCombo = nullptr;
    ColorSeletorWidget *m_colorSelector = nullptr;
    QAbstractButton *m_okButton = nullptr;
};

#endif // SUBSCRIBEICSDLG_H
