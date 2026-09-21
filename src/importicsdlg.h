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
 * 参考实现（dde-calendar 6.6.0）的导入只有一个入口：在日历列表里选「导入」，
 * 挑一个 .ics 文件，然后无条件新建一个日历把文件导进去（见
 * calendar-client/src/customWidget/jobtypelistview.cpp::slotImportScheduleType）。
 * 这里多给一个「导入到已有的日历」，进哪个日历由用户在弹窗里选。
 */

#ifndef IMPORTICSDLG_H
#define IMPORTICSDLG_H

#include "dde25/dcalendarddialog.h"
#include "schedule/calendarservice.h"

//DPushButton / DLineEdit 不能只写前置声明：DWIDGET_USE_NAMESPACE 会把
//Dtk::Widget::DPushButton 引进全局，跟前置声明的 ::DPushButton 撞成二义
//（同 colorpickerwidget.h 的处理）
#include <DLineEdit>
#include <DPushButton>

#include <QString>

DWIDGET_USE_NAMESPACE

class QComboBox;
class QLabel;
class QAbstractButton;

/**
 * @brief 「从 ICS 文件导入」弹窗：选文件 + 选导入到哪个日历。
 *
 * 弹窗自己完成导入（按「导入」时就写库了），结果由调用方读
 * importSucceeded()/importError() 决定提示什么。导入成功会由数据层发出
 * scheduleUpdate()/scheduleTypeUpdate()，调用方据此刷新。
 */
class CImportIcsDlg : public DCalendarDDialog
{
    Q_OBJECT
public:
    explicit CImportIcsDlg(QWidget *parent = nullptr);

    bool importSucceeded() const { return m_importSucceeded; }
    //失败原因（成功时为空）
    QString importError() const { return m_importError; }
    //导入成功后日程落在哪个日历里
    QString targetName() const { return m_targetName; }
    //文件里有多少个日程（读不了文件时为 0）
    int eventCount() const { return m_hints.eventCount; }

protected:
    void changeEvent(QEvent *event) override;

private:
    void initUI();
    void initConnection();
    void setTheMe(const int type);

    //按当前文件名/目标刷新名字预填、提示行和「导入」按钮的可用状态
    void updateAcceptState();
    //文件名变了就重新读一遍文件里的提示（名字、颜色、日程数）
    void slotFileChanged();
    void slotBrowse();
    void slotTargetChanged();
    void slotImport();

    CalendarService::IcsFileHints m_hints;
    //上一次按文件预填的名字，用户自己改过之后就不再覆盖
    QString m_lastSuggestedName;

    bool m_importSucceeded = false;
    QString m_importError;
    QString m_targetName;

    QLabel *m_fileLabel = nullptr;
    QLabel *m_targetLabel = nullptr;
    QLabel *m_nameLabel = nullptr;
    QLabel *m_infoLabel = nullptr;
    DLineEdit *m_fileEdit = nullptr;
    DLineEdit *m_nameEdit = nullptr;
    QComboBox *m_targetCombo = nullptr;
    DPushButton *m_browseButton = nullptr;
    QAbstractButton *m_okButton = nullptr;
};

#endif // IMPORTICSDLG_H
