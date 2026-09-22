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
 * Origin copyright bearer: 2015 - 2026 UnionTech Software Technology Co., Ltd.
 * This file is ported from DDE calendar tag 6.6.0
 * Minimal modification is applied to make the class build against
 * DTK2Widget-Qt6.
 * ----------------------------------------------------------------------------
 * 移植自 dde-calendar（src/calendar-client/src/customWidget/timeedit.*）。
 *
 * 差异：
 * 1. DTK2Widget 没有 DComboBox，基类换成 QComboBox。参考实现里 DComboBox 只是在
 *    showPopup() 里做了一次 setMaxVisibleItems(16)，本项目用不到；
 * 2. 构造函数原来读 CalendarManager::getInstance()->getTimeFormat() 并连
 *    signalTimeFormatChanged，本项目没有这个全局时间制式管理器，默认按
 *    24 小时制（"hh:mm"），需要时由调用方走 setTimeFormat()；
 * 3. 去掉 Qt5.11 的兼容分支和 commondef.h 日志宏。
 */

#ifndef TIMEEDIT_H
#define TIMEEDIT_H

#include "ccustomtimeedit.h"

#include <QComboBox>
#include <QTime>
#include <QTimeEdit>

class CTimeEdit : public QComboBox
{
    Q_OBJECT
public:
    explicit CTimeEdit(QWidget *parent = nullptr);
    ~CTimeEdit() override;

    /**
     * @brief setMineTime   设置下拉列表顶端时间
     * @param mineTime
     */
    void setMineTime(const QTime &mineTime);

    /**
     * @brief setTime       设置显示时间
     * @param time
     */
    void setTime(const QTime &time);

    /**
     * @brief getTime       获取时间
     * @return
     */
    QTime getTime();

    /**
     * @brief updateListItem        更新列表项
     * @param isShowTimeInterval     是否显示时间间隔
     */
    void updateListItem(bool isShowTimeInterval = false);

    /**
     * @brief setSelectItem     设置选中项目和scroll定位位置
     * @param time
     */
    void setSelectItem(const QTime &time);
public slots:
    void setTimeFormat(int value);
    //焦点获取效果绘制
    void slotFocusDraw(bool showFocus);


    /**
     * @brief slotSetPlainText      设置当前编辑框显示文本
     * @param arg
     */
    void slotSetPlainText(const QString &arg);

    /**
     * @brief slotActivated         Combobox选择项处理槽
     * @param arg
     */
    void slotActivated(const QString &arg);

    /**
     * @brief slotEditingFinished       编辑框编辑结束时，处理槽
     */
    void slotEditingFinished();
signals:
    void signalFocusOut();
    void signaleditingFinished();
private:
    void initUI();
    void initConnection();
protected:
    void showPopup() override;
    void focusInEvent(QFocusEvent *event) override;
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
private:
    QString m_timeFormat = "hh:mm";
    QTime m_time;
    CCustomTimeEdit *m_timeEdit = nullptr;
    bool m_hasFocus;
    QTime m_miniTime;
    bool m_isShowTimeInterval = false;
    QModelIndex scrollPosition;     //下拉列表需要滚动到的位置
};

#endif // TIMEEDIT_H
