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
 */

#ifndef SRC_ICSSUBSCRIPTIONDLG_H_
#define SRC_ICSSUBSCRIPTIONDLG_H_

#include "dde25/dcalendarddialog.h"
#include "schedule/calendarservice.h"

#include <DPushButton>

#include <QFrame>
#include <QHash>
#include <QString>
#include <QVector>

DWIDGET_USE_NAMESPACE

class QLabel;
class QScrollArea;
class QVBoxLayout;

class CIcsSubscriptionItem : public QFrame {
    Q_OBJECT
public:
    explicit CIcsSubscriptionItem(QWidget *parent = nullptr);

    void setData(const CalendarService::IcsSubscriptionInfo &info);
    QString typeID() const { return m_info.typeID; }

    /**
     * @brief setStatusText  用一句话盖掉状态行（「同步中…」/ 失败原因）
     * @param error          为 true 时用警示色显示
     */
    void setStatusText(const QString &text, bool error = false);

signals:
    void signalRefresh(const QString &typeID);
    void signalRemove(const QString &typeID);

protected:
    void changeEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QString syncStateText() const;
    QString currentStatusText() const;
    void applyTheme();
    void updateStatus();
    void updateElidedTexts();

    CalendarService::IcsSubscriptionInfo m_info;
    QString m_statusText;
    bool m_statusIsError = false;
    int m_appliedThemeType = -1;

    QLabel *m_colorDot = nullptr;
    QLabel *m_nameLabel = nullptr;
    QLabel *m_urlLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    DPushButton *m_refreshButton = nullptr;
    DPushButton *m_removeButton = nullptr;
};

class CIcsSubscriptionDlg : public DCalendarDDialog {
    Q_OBJECT
public:
    explicit CIcsSubscriptionDlg(QWidget *parent = nullptr);

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void slotAddSubscription();
    void slotImportFromFile();
    void slotRefreshAll();
    void slotRefreshOne(const QString &typeID);
    void slotRemove(const QString &typeID);
    void slotRefreshFinished(const QString &typeID, bool ok, const QString &error);
    void slotReload();

private:
    void initUI();
    void initConnection();
    void setTheMe(const int type);
    void rebuild();

    QWidget *m_itemContainer = nullptr;
    QVBoxLayout *m_itemLayout = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QLabel *m_emptyLabel = nullptr;
    QVector<CIcsSubscriptionItem *> m_items;
    QHash<QString, QString> m_statusOverrides;
    QHash<QString, bool> m_statusErrors;
};

#endif  // SRC_ICSSUBSCRIPTIONDLG_H_
