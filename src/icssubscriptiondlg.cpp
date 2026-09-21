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

#include "icssubscriptiondlg.h"

#include "dde25/dde25common.h"
#include "importicsdlg.h"
#include "subscribeicsdlg.h"

#include <DPushButton>

#include <QFontMetrics>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

namespace {

const int RowHeight = 68;
const int ContentWidth = 560;
const int ContentHeight = 300;

QColor dialogTextColor(int themeType, qreal alpha) {
    QColor color = (themeType == 2) ? QColor(Qt::white) : QColor(Qt::black);
    color.setAlphaF(alpha);
    return color;
}

QColor warningColor() {
    return QColor("#F25C5C");
}

QString intervalText(int minutes) {
    if (minutes >= 24 * 60) {
        return QCoreApplication::translate("CIcsSubscriptionItem", "%1 d").arg(minutes / (24 * 60));
    }
    if (minutes >= 60) {
        return QCoreApplication::translate("CIcsSubscriptionItem", "%1 h").arg(minutes / 60);
    }
    return QCoreApplication::translate("CIcsSubscriptionItem", "%1 min").arg(minutes);
}

} // namespace

CIcsSubscriptionItem::CIcsSubscriptionItem(QWidget *parent)
        : QFrame(parent) {
    setFixedHeight(RowHeight);
    setAttribute(Qt::WA_StyledBackground, true);

    m_colorDot = new QLabel(this);
    m_colorDot->setFixedSize(10, 10);

    m_nameLabel = new QLabel(this);
    m_urlLabel = new QLabel(this);
    m_statusLabel = new QLabel(this);

    QFont nameFont = m_nameLabel->font();
    nameFont.setPixelSize(13);
    m_nameLabel->setFont(nameFont);

    QFont smallFont = nameFont;
    smallFont.setPixelSize(11);
    m_urlLabel->setFont(smallFont);
    m_statusLabel->setFont(smallFont);

    m_urlLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    m_statusLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    m_refreshButton = new DPushButton(tr("Refresh"), this);
    m_refreshButton->setFixedSize(64, 28);
    m_refreshButton->setFocusPolicy(Qt::NoFocus);

    m_removeButton = new DPushButton(tr("Unsubscribe"), this);
    m_removeButton->setFixedSize(96, 28);
    m_removeButton->setFocusPolicy(Qt::NoFocus);

    connect(m_refreshButton, &DPushButton::clicked, this, [this] {
        emit signalRefresh(m_info.typeID);
    });
    connect(m_removeButton, &DPushButton::clicked, this, [this] {
        emit signalRemove(m_info.typeID);
    });

    QVBoxLayout *textLayout = new QVBoxLayout;
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(2);
    textLayout->addWidget(m_nameLabel);
    textLayout->addWidget(m_urlLabel);
    textLayout->addWidget(m_statusLabel);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(12, 8, 12, 8);
    mainLayout->setSpacing(10);
    mainLayout->addWidget(m_colorDot, 0, Qt::AlignVCenter);
    mainLayout->addLayout(textLayout, 1);
    mainLayout->addWidget(m_refreshButton, 0, Qt::AlignVCenter);
    mainLayout->addWidget(m_removeButton, 0, Qt::AlignVCenter);

    applyTheme();
}

void CIcsSubscriptionItem::setData(const CalendarService::IcsSubscriptionInfo &info) {
    m_info = info;

    QColor color(info.colorCode);
    if (!color.isValid()) {
        color = DDE25::systemActiveColor();
    }
    m_colorDot->setStyleSheet(QStringLiteral("QLabel { background-color: %1; border-radius: 5px; }")
        .arg(color.name()));

    m_nameLabel->setText(info.displayName);
    m_nameLabel->setToolTip(info.displayName);
    m_urlLabel->setToolTip(info.url);

    m_statusText.clear();
    m_statusIsError = false;
    updateStatus();
    updateElidedTexts();
}

void CIcsSubscriptionItem::setStatusText(const QString &text, bool error) {
    m_statusText = text;
    m_statusIsError = error;
    updateStatus();
    updateElidedTexts();
}

QString CIcsSubscriptionItem::syncStateText() const {
    QStringList parts;
    parts << (m_info.refreshIntervalMin <= 0
                      ? tr("No auto refresh")
                      : tr("Auto refresh every %1").arg(intervalText(m_info.refreshIntervalMin)));
    parts << (m_info.lastSync.isValid()
                      ? tr("Last synced %1").arg(m_info.lastSync.toString("yyyy-MM-dd hh:mm"))
                      : tr("Not synced yet"));

    return parts.join(QStringLiteral(" · "));
}

QString CIcsSubscriptionItem::currentStatusText() const {
    return m_statusText.isEmpty() ? syncStateText() : m_statusText;
}

void CIcsSubscriptionItem::updateStatus() {
    if (m_statusLabel == nullptr) {
        return;
    }

    const int themeType = DDE25::themeType();
    const QColor color = m_statusIsError ? warningColor() : dialogTextColor(themeType, 0.5);

    m_statusLabel->setToolTip(currentStatusText());
    m_statusLabel->setStyleSheet(QStringLiteral("QLabel { color: %1; }")
                                         .arg(color.name(QColor::HexArgb)));
}

void CIcsSubscriptionItem::updateElidedTexts() {
    if (m_urlLabel == nullptr) {
        return;
    }

    const QFontMetrics urlMetrics(m_urlLabel->font());
    //地址中段省略，域名和文件名都能看见
    m_urlLabel->setText(urlMetrics.elidedText(m_info.url, Qt::ElideMiddle, m_urlLabel->width()));

    //每次都从完整文字重新省略；直接读 m_statusLabel->text() 的话会把上一次的
    //省略结果再省略一遍，窗口一缩放文字就越来越短
    const QFontMetrics statusMetrics(m_statusLabel->font());
    m_statusLabel->setText(statusMetrics.elidedText(currentStatusText(),
                                                    Qt::ElideRight, m_statusLabel->width()));
}

void CIcsSubscriptionItem::applyTheme() {
    const int themeType = DDE25::themeType();

    if (m_colorDot == nullptr) {
        return;
    }

    //同一套主题下重复进来直接返回：这里的颜色全按 themeType 算，重设样式表
    //不会有任何变化，却会再触发一次调色板变化（见 changeEvent 里的说明）
    if (m_appliedThemeType == themeType) {
        return;
    }
    m_appliedThemeType = themeType;

    //行本身给一层很淡的底色，跟对话框背景区分开
    const QColor background = (themeType == 2) ? QColor(255, 255, 255, 16)
                                               : QColor(0, 0, 0, 10);
    setStyleSheet(QStringLiteral("CIcsSubscriptionItem { background-color: %1; border-radius: 6px; }")
                          .arg(background.name(QColor::HexArgb)));

    const QString nameStyle = QStringLiteral("QLabel { color: %1; }")
                                      .arg(dialogTextColor(themeType, 0.9).name(QColor::HexArgb));
    const QString urlStyle = QStringLiteral("QLabel { color: %1; }")
                                     .arg(dialogTextColor(themeType, 0.6).name(QColor::HexArgb));
    m_nameLabel->setStyleSheet(nameStyle);
    //状态行可能是警示色，颜色在 updateStatus() 里单独设
    m_urlLabel->setStyleSheet(urlStyle);
    updateStatus();
}

void CIcsSubscriptionItem::changeEvent(QEvent *event) {
    if (event->type() == QEvent::ApplicationPaletteChange) {
        applyTheme();
    }

    QFrame::changeEvent(event);
}

void CIcsSubscriptionItem::resizeEvent(QResizeEvent *event) {
    QFrame::resizeEvent(event);
    updateElidedTexts();
}

CIcsSubscriptionDlg::CIcsSubscriptionDlg(QWidget *parent)
        : DCalendarDDialog(parent) {
    setContentsMargins(0, 0, 0, 0);
    setTitle(tr("Manage Online Calendars"));
    initUI();
    initConnection();
    setTheMe(DDE25::themeType());
    rebuild();
    resize(620, 470);
}

void CIcsSubscriptionDlg::initUI() {
    m_emptyLabel = new QLabel(tr("No online calendar subscribed yet"));
    m_emptyLabel->setAlignment(Qt::AlignCenter);

    m_itemContainer = new QWidget;
    m_itemContainer->setAutoFillBackground(false);
    m_itemLayout = new QVBoxLayout(m_itemContainer);
    m_itemLayout->setContentsMargins(0, 0, 0, 0);
    m_itemLayout->setSpacing(6);
    m_itemLayout->addWidget(m_emptyLabel, 0, Qt::AlignHCenter);
    m_itemLayout->addStretch();

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    m_scrollArea->viewport()->setAutoFillBackground(false);
    m_scrollArea->setWidget(m_itemContainer);

    QWidget *content = new QWidget(this);
    content->setFixedSize(ContentWidth, ContentHeight);
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->addWidget(m_scrollArea);
    addContent(content, Qt::AlignCenter);

    //添加按钮
    addButton(tr("Add Subscription", "button"), false, DDialog::ButtonRecommend);
    addButton(tr("Import from File", "button"));
    addButton(tr("Refresh All", "button"));
    addButton(tr("Close", "button"));

    //「添加订阅」「从文件导入」「全部刷新」点完要继续留在弹窗里，所以不能点一下
    //就把弹窗关掉
    setOnButtonClickedClose(false);

    const int count = buttonCount();
    for (int i = 0; i < count; i++) {
        getButton(i)->setFixedSize(140, 36);
    }
}

void CIcsSubscriptionDlg::initConnection() {
    //0 = 添加订阅，1 = 从文件导入，2 = 全部刷新，3 = 关闭，顺序见 initUI()
    connect(this, &DDialog::buttonClicked, this, [this](int index, const QString &text) {
        Q_UNUSED(text)
        if (index == 0) {
            slotAddSubscription();
            return;
        }
        if (index == 1) {
            slotImportFromFile();
            return;
        }
        if (index == 2) {
            slotRefreshAll();
            return;
        }
        reject();
    });

    CalendarService *service = CalendarService::instance();
    //订阅增删 -> scheduleTypeUpdate；同步入库 -> scheduleUpdate（顺带更新 lastSync）
    connect(service, &CalendarService::scheduleTypeUpdate, this, &CIcsSubscriptionDlg::slotReload);
    connect(service, &CalendarService::scheduleUpdate, this, &CIcsSubscriptionDlg::slotReload);
    connect(service, &CalendarService::icsRefreshFinished,
            this, &CIcsSubscriptionDlg::slotRefreshFinished);
}

void CIcsSubscriptionDlg::setTheMe(const int type) {
    if (m_emptyLabel == nullptr) {
        return;
    }

    m_emptyLabel->setStyleSheet(QStringLiteral("QLabel { color: %1; }")
                                        .arg(dialogTextColor(type, 0.4).name(QColor::HexArgb)));
}

void CIcsSubscriptionDlg::changeEvent(QEvent *event) {
    if (event->type() == QEvent::PaletteChange
            || event->type() == QEvent::ApplicationPaletteChange) {
        setTheMe(DDE25::themeType());
    }

    DCalendarDDialog::changeEvent(event);
}

void CIcsSubscriptionDlg::rebuild() {
    for (CIcsSubscriptionItem *item : m_items) {
        m_itemLayout->removeWidget(item);
        item->deleteLater();
    }
    m_items.clear();

    const QVector<CalendarService::IcsSubscriptionInfo> list =
            CalendarService::instance()->getIcsSubscriptionList();

    m_emptyLabel->setVisible(list.isEmpty());

    const int insertAt = m_itemLayout->count() - 1;
    int offset = 0;
    for (const CalendarService::IcsSubscriptionInfo &info : list) {
        CIcsSubscriptionItem *item = new CIcsSubscriptionItem(m_itemContainer);
        item->setData(info);
        if (m_statusOverrides.contains(info.typeID)) {
            item->setStatusText(m_statusOverrides.value(info.typeID),
                                m_statusErrors.value(info.typeID, false));
        }
        connect(item, &CIcsSubscriptionItem::signalRefresh,
                this, &CIcsSubscriptionDlg::slotRefreshOne);
        connect(item, &CIcsSubscriptionItem::signalRemove,
                this, &CIcsSubscriptionDlg::slotRemove);
        m_itemLayout->insertWidget(insertAt + offset, item);
        m_items.append(item);
        ++offset;
    }
}

void CIcsSubscriptionDlg::slotReload() {
    rebuild();
}

void CIcsSubscriptionDlg::slotAddSubscription() {
    CSubscribeIcsDlg dlg(this);
    if (dlg.exec() != DDialog::Accepted) {
        return;
    }

    CalendarService *service = CalendarService::instance();
    const QString typeID = service->subscribeIcs(dlg.url(), dlg.displayName(),
                                                 dlg.refreshIntervalMin());
    if (typeID.isEmpty()) {
        DDialog prompt(this);
        prompt.setIcon(QIcon::fromTheme("dialog-warning"), QSize(32, 32));
        prompt.setMessage(tr("Failed to add the subscription"));
        prompt.addButton(tr("OK", "button"), true, DDialog::ButtonNormal);
        prompt.exec();
        return;
    }

    //拉取是异步的，先把状态行标成「同步中」，结果回来再改（见 slotRefreshFinished）
    m_statusOverrides.insert(typeID, tr("Syncing..."));
    m_statusErrors.insert(typeID, false);
    rebuild();
}

void CIcsSubscriptionDlg::slotImportFromFile() {
    CImportIcsDlg dlg(this);
    if (dlg.exec() != DDialog::Accepted) {
        return;
    }

    //导入成功/失败都给个结果：清单本身由 scheduleTypeUpdate/scheduleUpdate
    // 触发 slotReload 刷新，这里只负责告诉用户成了没有
    DDialog prompt(this);
    prompt.addButton(tr("OK", "button"), true, DDialog::ButtonNormal);
    if (dlg.importSucceeded()) {
        prompt.setMessage(tr("Imported %1 events into \"%2\"")
                                  .arg(dlg.eventCount())
                                  .arg(dlg.targetName()));
    } else {
        prompt.setIcon(QIcon::fromTheme("dialog-warning"), QSize(32, 32));
        prompt.setMessage(dlg.importError().isEmpty() ? tr("Failed to import the file")
                                                      : dlg.importError());
    }
    prompt.exec();
}

void CIcsSubscriptionDlg::slotRefreshAll() {
    CalendarService *service = CalendarService::instance();
    const QVector<CalendarService::IcsSubscriptionInfo> list = service->getIcsSubscriptionList();
    if (list.isEmpty()) {
        return;
    }

    for (const CalendarService::IcsSubscriptionInfo &info : list) {
        m_statusOverrides.insert(info.typeID, tr("Syncing..."));
        m_statusErrors.insert(info.typeID, false);
    }
    rebuild();

    //「全部刷新」是用户主动点的，忽略刷新间隔
    service->refreshAllIcs(true);
}

void CIcsSubscriptionDlg::slotRefreshOne(const QString &typeID) {
    m_statusOverrides.insert(typeID, tr("Syncing..."));
    m_statusErrors.insert(typeID, false);
    rebuild();

    CalendarService::instance()->refreshIcs(typeID);
}

void CIcsSubscriptionDlg::slotRefreshFinished(const QString &typeID, bool ok, const QString &error) {
    if (ok) {
        m_statusOverrides.remove(typeID);
        m_statusErrors.remove(typeID);
    } else {
        m_statusOverrides.insert(typeID, tr("Sync failed: %1")
                                             .arg(error.isEmpty() ? tr("unknown error") : error));
        m_statusErrors.insert(typeID, true);
    }

    rebuild();
}

void CIcsSubscriptionDlg::slotRemove(const QString &typeID) {
    QString name = typeID;
    const QVector<CalendarService::IcsSubscriptionInfo> list =
            CalendarService::instance()->getIcsSubscriptionList();
    for (const CalendarService::IcsSubscriptionInfo &info : list) {
        if (info.typeID == typeID) {
            name = info.displayName;
            break;
        }
    }

    DDialog prompt(this);
    prompt.setIcon(QIcon::fromTheme("dialog-warning"), QSize(32, 32));
    prompt.setTitle(tr("Unsubscribe \"%1\"?").arg(name));
    prompt.setMessage(tr("The events already synced from this calendar will be deleted."));
    prompt.addButton(tr("Cancel", "button"));
    prompt.addButton(tr("Unsubscribe", "button"), false, DDialog::ButtonWarning);

    if (prompt.exec() != 1) {
        return;
    }

    CalendarService::instance()->unsubscribeIcs(typeID);
    m_statusOverrides.remove(typeID);
    m_statusErrors.remove(typeID);
    rebuild();
}
