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

#include "subscribeicsdlg.h"

#include "dde25/colorseletorwidget.h"
#include "dde25/dde25common.h"

#include <QAbstractButton>
#include <QComboBox>
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QUrl>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

namespace {

//内容区宽度：跟管理弹窗（CIcsSubscriptionDlg）的列表宽度对齐
const int ContentWidth = 400;
//输入框/下拉框高度
const int FieldHeight = 36;

//订阅地址只认 http/https。数据层是用 QNetworkAccessManager 拉的，别的协议
//（file:// 之类）它拉不动，在界面上先挡掉，比拉取失败再报错清楚。
bool isAcceptableUrl(const QString &text)
{
    const QUrl url(text);
    if (!url.isValid() || url.host().isEmpty()) {
        return false;
    }

    const QString scheme = url.scheme().toLower();
    return scheme == QLatin1String("http") || scheme == QLatin1String("https");
}

//库里存的间隔不一定是下拉框里那几档（老数据、别处写进来的值），编辑时按这个
//拼一个条目显示出来，免得选不中就被静默改成默认档
QString intervalText(int minutes)
{
    if (minutes % (24 * 60) == 0) {
        return QCoreApplication::translate("CSubscribeIcsDlg", "%1 d").arg(minutes / (24 * 60));
    }
    if (minutes % 60 == 0) {
        return QCoreApplication::translate("CSubscribeIcsDlg", "%1 h").arg(minutes / 60);
    }
    return QCoreApplication::translate("CSubscribeIcsDlg", "%1 min").arg(minutes);
}

} // namespace

CSubscribeIcsDlg::CSubscribeIcsDlg(QWidget *parent)
    : DCalendarDDialog(parent)
{
    setContentsMargins(0, 0, 0, 0);
    setTitle(tr("Subscribe to Online Calendar"));
    initUI();
    initConnection();
    setTheMe(DDE25::themeType());
    //DDialog::updateSize() 只在没被手动 resize 过时才自己算尺寸，所以这里给足：
    //标题 + 四组标签输入框 + 按钮，矮了会把内容压扁
    resize(440, 420);
}

void CSubscribeIcsDlg::initUI()
{
    //标签 + 输入框，竖向排四组
    m_urlLabel = new QLabel(tr("Address"));
    m_nameLabel = new QLabel(tr("Name"));
    m_intervalLabel = new QLabel(tr("Refresh Interval"));
    m_colorLabel = new QLabel(tr("Color"));

    m_urlEdit = new DLineEdit;
    m_urlEdit->setPlaceholderText(QStringLiteral("https://example.com/calendar.ics"));
    m_urlEdit->setFixedHeight(FieldHeight);
    m_urlEdit->setClearButtonEnabled(true);

    m_nameEdit = new DLineEdit;
    m_nameEdit->setPlaceholderText(tr("Optional, the address is used when left empty"));
    m_nameEdit->setFixedHeight(FieldHeight);
    m_nameEdit->setClearButtonEnabled(true);

    m_intervalCombo = new QComboBox;
    m_intervalCombo->setFixedHeight(FieldHeight);
    //0 是数据层的「不自动刷新」，其余是刷新间隔（分钟）
    m_intervalCombo->addItem(tr("No auto refresh"), 0);
    m_intervalCombo->addItem(tr("Every 15 minutes"), 15);
    m_intervalCombo->addItem(tr("Every hour"), 60);
    m_intervalCombo->addItem(tr("Every 6 hours"), 6 * 60);
    m_intervalCombo->addItem(tr("Every day"), 24 * 60);
    m_intervalCombo->addItem(tr("Every week"), 7 * 24 * 60);
    m_intervalCombo->setCurrentIndex(4);

    //颜色：跟新建日历、日程弹窗用的是同一个控件（九色 + 自定义取色器）
    m_colorSelector = new ColorSeletorWidget(this);
    m_colorSelector->resetColorButton();

    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(6);
    contentLayout->addWidget(m_urlLabel);
    contentLayout->addWidget(m_urlEdit);
    contentLayout->addSpacing(6);
    contentLayout->addWidget(m_nameLabel);
    contentLayout->addWidget(m_nameEdit);
    contentLayout->addSpacing(6);
    contentLayout->addWidget(m_intervalLabel);
    contentLayout->addWidget(m_intervalCombo);
    contentLayout->addSpacing(6);
    contentLayout->addWidget(m_colorLabel);
    contentLayout->addWidget(m_colorSelector);
    contentLayout->addStretch();

    m_gwi = new DFrame(this);
    m_gwi->setFrameShape(QFrame::NoFrame);
    m_gwi->setFixedWidth(ContentWidth);
    m_gwi->setLayout(contentLayout);
    addContent(m_gwi, Qt::AlignCenter);

    //添加按钮
    addButton(tr("Cancel", "button"));
    addButton(tr("Subscribe", "button"), false, DDialog::ButtonRecommend);

    //「订阅」要校验地址之后再关，所以不能点一下就关（参考 scheduledlg 的做法）
    setOnButtonClickedClose(false);

    //只定高不定宽：宽度由 DDialog 的按钮行平分，两个按钮各占一半（写死 140 的话
    //按钮会缩在中间，两边各空一条——同 CScheduleCtrlDlg::addPushButton 的说明）
    const int buttonCount = this->buttonCount();
    for (int i = 0; i < buttonCount; i++) {
        QAbstractButton *button = getButton(i);
        button->setFixedHeight(36);
    }
    m_okButton = getButton(buttonCount - 1);
}

void CSubscribeIcsDlg::initConnection()
{
    connect(m_urlEdit, &DLineEdit::textChanged, this, [this] { updateAcceptState(); });

    connect(this, &DDialog::buttonClicked, this, [this](int index, const QString &text) {
        Q_UNUSED(text)
        //0 = 取消，1 = 订阅，顺序见 initUI()
        if (index == 1 && m_okButton != nullptr && m_okButton->isEnabled()) {
            accept();
            return;
        }
        reject();
    });

    updateAcceptState();
}

void CSubscribeIcsDlg::setTheMe(const int type)
{
    //标签文字颜色：浅色主题下黑、深色主题下白，都带一点透明度
    QColor labelColor = (type == 2) ? QColor(Qt::white) : QColor(Qt::black);
    labelColor.setAlphaF(0.9);
    const QString style = QStringLiteral("QLabel { color: %1; }")
                                  .arg(labelColor.name(QColor::HexArgb));

    if (m_urlLabel != nullptr) {
        m_urlLabel->setStyleSheet(style);
        m_nameLabel->setStyleSheet(style);
        m_intervalLabel->setStyleSheet(style);
        m_colorLabel->setStyleSheet(style);
    }
}

void CSubscribeIcsDlg::changeEvent(QEvent *event)
{
    //参考实现连的是 DGuiApplicationHelper::themeTypeChanged，DTK2Widget 下没有
    //这个信号，改成跟着调色板变化重新取色（同 schedulectrldlg）
    if (event->type() == QEvent::PaletteChange
            || event->type() == QEvent::ApplicationPaletteChange) {
        setTheMe(DDE25::themeType());
    }

    DCalendarDDialog::changeEvent(event);
}

void CSubscribeIcsDlg::updateAcceptState()
{
    if (m_okButton == nullptr) {
        return;
    }

    m_okButton->setEnabled(isAcceptableUrl(url()));
}

QString CSubscribeIcsDlg::url() const
{
    return m_urlEdit->text().trimmed();
}

QString CSubscribeIcsDlg::displayName() const
{
    return m_nameEdit->text().trimmed();
}

int CSubscribeIcsDlg::refreshIntervalMin() const
{
    return m_intervalCombo->currentData().toInt();
}

QString CSubscribeIcsDlg::colorCode() const
{
    const DTypeColor::Ptr color = m_colorSelector->getSelectedColorInfo();
    return color.isNull() ? QString() : color->colorCode();
}

void CSubscribeIcsDlg::setEditData(const CalendarService::IcsSubscriptionInfo &info)
{
    setTitle(tr("Edit Subscription"));
    //按钮文字跟着改：还写「订阅」的话看着像又要新建一个
    if (m_okButton != nullptr) {
        m_okButton->setText(tr("Save", "button"));
    }

    m_urlEdit->setText(info.url);
    m_nameEdit->setText(info.displayName);
    m_nameEdit->setPlaceholderText(info.url);

    //间隔对不上现成档位（老数据、别处写进来的值）就补一个条目，不然选不中会被
    //静默改成别的档
    int index = m_intervalCombo->findData(info.refreshIntervalMin);
    if (index < 0) {
        m_intervalCombo->addItem(intervalText(info.refreshIntervalMin), info.refreshIntervalMin);
        index = m_intervalCombo->count() - 1;
    }
    m_intervalCombo->setCurrentIndex(index);

    //按色值选中对应的色块；色值对不上任何一块（比如类型颜色被删了）时
    //setSelectedColor() 会把它加成一个自定义色块，也算选中了
    if (!info.colorCode.isEmpty()) {
        DTypeColor color;
        color.setColorCode(info.colorCode);
        m_colorSelector->setSelectedColor(color);
    }

    updateAcceptState();
}
