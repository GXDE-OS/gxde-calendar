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
 * 本文件重写自 dde-calendar（src/calendar-client/src/customWidget/jobtypecombobox.*），
 * 保留原有的交互（下拉项带色块、“新建日程类型”行、键盘上下移动焦点、
 * 高亮假选项、进入编辑态后回车提交）与对外接口，只替换底层依赖：
 *
 * 1. DTK2Widget 没有 DComboBox，基类换成 QComboBox；
 * 2. DTK2Widget 没有 DAlertControl，改用它本来要挂在 QLineEdit 上的 DLineEdit：
 *    进入编辑态时 QComboBox::setLineEdit(new DLineEdit) 顶掉默认的 QLineEdit，
 *    告警就落在 DLineEdit 自己身上（它也提供 alertChanged/showAlertMessage）。
 *    因此 setAlertMessageAlignment() 无处可落，删掉；
 *    showAlertMessage() 的 follower 重载（DAlertControl 的浮动提示定位）同样删掉，
 *    只保留单参数版本——调用方也只用这一版；
 * 3. 无账户系统：updateJobType() 不吃 AccountItem，日历类型直接取
 *    CalendarService::getScheduleTypeList()；CalDAV 相关分支
 *    （只读日历禁用新建类型、只读/可写日历的默认选中）整体去掉；
 * 4. 加号按钮 CPushButton 见 cpushbutton.h；分隔线仍用 DPushButton。
 */

#ifndef JOBTYPECOMBOBOX_H
#define JOBTYPECOMBOBOX_H

#include "cpushbutton.h"
#include "dscheduletype.h"

#include <DLineEdit>

#include <QComboBox>

DWIDGET_USE_NAMESPACE

class JobTypeComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit JobTypeComboBox(QWidget *parent = nullptr);
    ~JobTypeComboBox() override;
    void updateJobType();
    QString getCurrentJobTypeNo();
    void setCurrentJobTypeNo(const QString &strJobTypeNo);
    //列表里有没有这个日历（编辑弹窗用来判断日程原本的类型还能不能选上）
    bool containsJobTypeNo(const QString &strJobTypeNo) const;
    //把一个不在列表里的日历插到指定位置。编辑弹窗用：日程所属的类型可能不展示在
    //列表里（历史数据把日程建进了不展示的节假日类型），得如实显示出来
    void insertJobTypeItem(int idx, const DScheduleType::Ptr &type);

    void setAlert(bool isAlert);
    bool isAlert() const;
    void showAlertMessage(const QString &text, int duration = 3000);
    void hideAlertMessage();

    int getCurrentEditPosition() const;

signals:
    void signalAddTypeBtnClicked();
    void alertChanged(bool alert) const;
    void editingFinished();
public slots:

protected slots:
    void slotBtnAddItemClicked();
    void slotEditingFinished();
    void slotEditCursorPositionChanged(int oldPos, int newPos);

protected:
    void showPopup() override;
    bool eventFilter(QObject *, QEvent *) override;

private:
    void initUI();
    void addJobTypeItem(int idx, QString strColorHex, QString strJobType);
    void addCustomWidget(QFrame *);
    void setItemSelectable(bool status);

    //进入编辑态后 lineEdit() 就是我们塞进去的 DLineEdit，未进入编辑态时返回 nullptr
    DLineEdit *alertLineEdit() const;

private:
    QWidget *m_customWidget {nullptr};
    CPushButton *m_addBtn {nullptr};
    DScheduleType::List m_lstJobType;
    int m_hoverSelectedIndex = -1; //鼠标悬停的选项下标
    int m_itemNumIndex = 0; //item数量
    QLineEdit *m_lineEdit {nullptr};
    int m_oldPos = 0;
    int m_newPos = 0;
};

#endif // JOBTYPECOMBOBOX_H
