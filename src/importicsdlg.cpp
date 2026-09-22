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

#include "importicsdlg.h"

#include "dde25/dde25common.h"

#include <dframe.h>

#include <QAbstractButton>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QSet>
#include <QSettings>
#include <QStandardPaths>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

namespace {

//内容区宽度：跟同级的 CSubscribeIcsDlg 表单对齐
const int ContentWidth = 400;
//输入框/下拉框高度
const int FieldHeight = 36;
//「新建日历」在下拉框里用空串当数据，代表「不落到已有日历上」
const QString NewCalendarData;

//记住上次选文件的目录，默认「文档」（参考实现的 icsDialogDirectory 也是这么做的）
QString lastImportDir()
{
    const QString saved = QSettings().value(QStringLiteral("ics_import_dir")).toString();
    if (!saved.isEmpty() && QFileInfo::exists(saved)) {
        return saved;
    }

    const QString documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    return documents.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                               : documents;
}

void rememberImportDir(const QString &filePath)
{
    QSettings().setValue(QStringLiteral("ics_import_dir"), QFileInfo(filePath).absolutePath());
}

} // namespace

CImportIcsDlg::CImportIcsDlg(QWidget *parent)
    : DCalendarDDialog(parent)
{
    setContentsMargins(0, 0, 0, 0);
    setTitle(tr("Import from ICS File"));
    initUI();
    initConnection();
    setTheMe(DDE25::themeType());
    //同 CSubscribeIcsDlg：尺寸由这里定死，给足高度让提示行和按钮都放得下
    resize(480, 380);
}

void CImportIcsDlg::initUI()
{
    m_fileLabel = new QLabel(tr("File"));
    m_targetLabel = new QLabel(tr("Import to"));
    m_nameLabel = new QLabel(tr("Calendar Name"));

    m_fileEdit = new DLineEdit;
    m_fileEdit->setPlaceholderText(tr("Path to a .ics file"));
    m_fileEdit->setFixedHeight(FieldHeight);
    m_fileEdit->setClearButtonEnabled(true);

    m_browseButton = new DPushButton(tr("Browse"), this);
    m_browseButton->setFixedSize(80, FieldHeight);
    m_browseButton->setFocusPolicy(Qt::NoFocus);

    //文件和「浏览」并排
    QHBoxLayout *fileLayout = new QHBoxLayout;
    fileLayout->setContentsMargins(0, 0, 0, 0);
    fileLayout->setSpacing(8);
    fileLayout->addWidget(m_fileEdit, 1);
    fileLayout->addWidget(m_browseButton, 0);

    m_targetCombo = new QComboBox;
    m_targetCombo->setFixedHeight(FieldHeight);
    //已有的日历在前（订阅来的日历是只读的，不列出来），最后一项是新建
    const QSet<QString> subscribedTypeIDs = [] {
        QSet<QString> ids;
        const QVector<CalendarService::IcsSubscriptionInfo> subs =
                CalendarService::instance()->getIcsSubscriptionList();
        for (const CalendarService::IcsSubscriptionInfo &info : subs) {
            ids.insert(info.typeID);
        }
        return ids;
    }();
    const DScheduleType::List types = CalendarService::instance()->getScheduleTypeList();
    for (const DScheduleType::Ptr &type : types) {
        if (type.isNull() || subscribedTypeIDs.contains(type->typeID())) {
            continue;
        }
        m_targetCombo->addItem(type->displayName(), type->typeID());
    }
    m_targetCombo->addItem(tr("New calendar"), NewCalendarData);
    //默认导入到本地日历，而不是列表第一项——第一项是「Work」这类参考实现自带的
    //类型，不是用户的日历
    const int localIndex = m_targetCombo->findData(CalendarService::instance()->getLocalTypeID());
    if (localIndex >= 0) {
        m_targetCombo->setCurrentIndex(localIndex);
    }

    m_nameEdit = new DLineEdit;
    m_nameEdit->setPlaceholderText(tr("Optional, the file name is used when left empty"));
    m_nameEdit->setFixedHeight(FieldHeight);
    m_nameEdit->setClearButtonEnabled(true);

    m_infoLabel = new QLabel;
    m_infoLabel->setWordWrap(true);

    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(6);
    contentLayout->addWidget(m_fileLabel);
    contentLayout->addLayout(fileLayout);
    contentLayout->addSpacing(6);
    contentLayout->addWidget(m_targetLabel);
    contentLayout->addWidget(m_targetCombo);
    contentLayout->addSpacing(6);
    contentLayout->addWidget(m_nameLabel);
    contentLayout->addWidget(m_nameEdit);
    contentLayout->addSpacing(6);
    contentLayout->addWidget(m_infoLabel);
    contentLayout->addStretch();

    DFrame *content = new DFrame(this);
    content->setFrameShape(QFrame::NoFrame);
    content->setFixedWidth(ContentWidth);
    content->setLayout(contentLayout);
    addContent(content, Qt::AlignCenter);

    //添加按钮
    addButton(tr("Cancel", "button"));
    addButton(tr("Import", "button"), false, DDialog::ButtonRecommend);

    //「导入」要先校验文件和日历名，不能点一下就关（参考 scheduledlg 的做法）
    setOnButtonClickedClose(false);

    //只定高不定宽：宽度由 DDialog 的按钮行平分，两个按钮各占一半（同
    //CSubscribeIcsDlg::initUI 的说明）
    const int buttonCount = this->buttonCount();
    for (int i = 0; i < buttonCount; i++) {
        getButton(i)->setFixedHeight(36);
    }
    m_okButton = getButton(buttonCount - 1);
}

void CImportIcsDlg::initConnection()
{
    connect(m_fileEdit, &DLineEdit::textChanged, this, &CImportIcsDlg::slotFileChanged);
    connect(m_browseButton, &DPushButton::clicked, this, &CImportIcsDlg::slotBrowse);
    connect(m_targetCombo, &QComboBox::currentIndexChanged, this, &CImportIcsDlg::slotTargetChanged);
    connect(m_nameEdit, &DLineEdit::textChanged, this, [this] { updateAcceptState(); });

    connect(this, &DDialog::buttonClicked, this, [this](int index, const QString &text) {
        Q_UNUSED(text)
        //0 = 取消，1 = 导入，顺序见 initUI()
        if (index == 1 && m_okButton != nullptr && m_okButton->isEnabled()) {
            slotImport();
            return;
        }
        reject();
    });

    slotTargetChanged();
}

void CImportIcsDlg::setTheMe(const int type)
{
    //标签文字颜色：浅色主题下黑、深色主题下白，都带一点透明度
    QColor labelColor = (type == 2) ? QColor(Qt::white) : QColor(Qt::black);
    labelColor.setAlphaF(0.9);
    const QString labelStyle = QStringLiteral("QLabel { color: %1; }")
                                       .arg(labelColor.name(QColor::HexArgb));

    if (m_fileLabel == nullptr) {
        return;
    }

    m_fileLabel->setStyleSheet(labelStyle);
    m_targetLabel->setStyleSheet(labelStyle);
    m_nameLabel->setStyleSheet(labelStyle);
    //提示行单独上色：读不了文件时是警示色，见 updateAcceptState()
}

void CImportIcsDlg::changeEvent(QEvent *event)
{
    //参考实现连的是 DGuiApplicationHelper::themeTypeChanged，DTK2Widget 下没有
    //这个信号，改成跟着调色板变化重新取色（同 CSubscribeIcsDlg）
    if (event->type() == QEvent::PaletteChange
            || event->type() == QEvent::ApplicationPaletteChange) {
        setTheMe(DDE25::themeType());
    }

    DCalendarDDialog::changeEvent(event);
}

void CImportIcsDlg::slotFileChanged()
{
    const QString filePath = m_fileEdit->text().trimmed();
    m_hints = CalendarService::instance()->readIcsFileHints(filePath);

    //文件里有推荐的名字就拿来预填，但用户自己改过之后就不再覆盖
    const QString current = m_nameEdit->text().trimmed();
    if (current.isEmpty() || current == m_lastSuggestedName) {
        m_lastSuggestedName = m_hints.name;
        m_nameEdit->setText(m_hints.name);
    }

    updateAcceptState();
}

void CImportIcsDlg::slotBrowse()
{
    const QString nameFilter = tr("ICS files") + QStringLiteral(" (*.ics)");
    const QString filePath = QFileDialog::getOpenFileName(this, tr("Select an ICS File"),
                                                          lastImportDir(),
                                                          nameFilter);
    if (filePath.isEmpty()) {
        return;
    }

    rememberImportDir(filePath);
    m_fileEdit->setText(filePath);
}

void CImportIcsDlg::slotTargetChanged()
{
    //只有「新建日历」才需要填名字，导进已有日历不改它的名字
    const bool isNewCalendar = m_targetCombo->currentData().toString() == NewCalendarData;
    m_nameLabel->setEnabled(isNewCalendar);
    m_nameEdit->setEnabled(isNewCalendar);

    updateAcceptState();
}

void CImportIcsDlg::updateAcceptState()
{
    const bool isNewCalendar = m_targetCombo->currentData().toString() == NewCalendarData;
    const bool nameOk = !isNewCalendar || !m_nameEdit->text().trimmed().isEmpty();

    if (m_okButton != nullptr) {
        m_okButton->setEnabled(m_hints.valid && nameOk);
    }

    if (m_infoLabel == nullptr) {
        return;
    }

    const int themeType = DDE25::themeType();
    QColor color = (themeType == 2) ? QColor(Qt::white) : QColor(Qt::black);
    color.setAlphaF(0.5);

    if (m_fileEdit->text().trimmed().isEmpty()) {
        m_infoLabel->setText(tr("Choose an .ics file to import"));
    } else if (!m_hints.valid) {
        //读不了文件：可能是路径不对，也可能不是 ICS
        color = QColor("#F25C5C");
        m_infoLabel->setText(tr("Cannot read this file"));
    } else {
        m_infoLabel->setText(tr("%1 events in this file").arg(m_hints.eventCount));
    }

    m_infoLabel->setStyleSheet(QStringLiteral("QLabel { color: %1; }")
                                       .arg(color.name(QColor::HexArgb)));
}

void CImportIcsDlg::slotImport()
{
    m_importSucceeded = false;
    m_importError.clear();
    m_targetName.clear();

    const QString filePath = m_fileEdit->text().trimmed();
    CalendarService *service = CalendarService::instance();

    QString typeID = m_targetCombo->currentData().toString();
    //新建日历：导入会先按文件里的提示建一个类型，再导进去
    const bool isNewCalendar = typeID == NewCalendarData;
    if (isNewCalendar) {
        typeID = service->createUserScheduleType(m_nameEdit->text().trimmed(), m_hints.colorCode);
        if (typeID.isEmpty()) {
            m_importError = tr("Failed to create the calendar");
            accept();
            return;
        }
    }

    //导进新建的日历要清空（新建的本来就是空的，清一下也无妨）；导进已有的日历
    //不能清，否则会把人家原有日程删掉
    const bool ok = service->importSchedule(filePath, typeID, isNewCalendar);
    if (!ok) {
        //类型已经建出来了，但一个日程都没导进去，回滚掉免得留一个空日历
        if (isNewCalendar) {
            service->deleteScheduleTypeByID(typeID);
        }
        m_importError = tr("Failed to import the file");
        accept();
        return;
    }

    const DScheduleType::Ptr type = service->getScheduleTypeByID(typeID);
    m_targetName = type.isNull() ? m_nameEdit->text().trimmed() : type->displayName();
    m_importSucceeded = true;
    accept();
}
