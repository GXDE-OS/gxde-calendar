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
 */

#include "viewswitcher.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QPushButton>

ViewSwitcher::ViewSwitcher(QWidget *parent)
    : QWidget(parent) {
    setObjectName("ViewSwitcher");
    setAttribute(Qt::WA_StyledBackground, true);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(0);

    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);

    connect(m_group, &QButtonGroup::idClicked, this,
        &ViewSwitcher::currentChanged);

    // 容器背景、边框、文字色随深浅主题；运行时切换在 changeEvent 里再调一次
    applyTheme();
}

void ViewSwitcher::setLabels(const QStringList &labels) {
    for (QPushButton *button : m_buttons) {
        m_group->removeButton(button);
        delete button;
    }
    m_buttons.clear();

    QHBoxLayout *layout = qobject_cast<QHBoxLayout *>(this->layout());
    int id = 0;
    for (const QString &label : labels) {
        QPushButton *button = new QPushButton(label, this);
        button->setCheckable(true);
        button->setFocusPolicy(Qt::NoFocus);
        button->setCursor(Qt::PointingHandCursor);

        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        layout->addWidget(button, 1);
        m_group->addButton(button, id++);
        m_buttons.append(button);
    }

    if (!m_buttons.isEmpty()) {
        m_buttons.first()->setChecked(true);
    }
}

void ViewSwitcher::setCurrentIndex(int index) {
    if (index < 0 || index >= m_buttons.size() || index == currentIndex()) {
        return;
    }

    m_buttons.at(index)->setChecked(true);
    emit currentChanged(index);
}

int ViewSwitcher::currentIndex() const {
    return m_group->checkedId();
}

void ViewSwitcher::changeEvent(QEvent *event)
{
    // 跟随系统调色板（深浅主题）变化，刷新分段按钮配色，
    // 做法同各对话框的 setTheMe：监听 PaletteChange / ApplicationPaletteChange。
    if (event->type() == QEvent::PaletteChange
            || event->type() == QEvent::ApplicationPaletteChange) {
        // 重入保护：applyTheme() 里的 setStyleSheet() 会令 Qt 重设本控件调色板，
        // 进而再次派发 PaletteChange，若再次进入 applyTheme 会无限递归直至栈溢出。
        if (!m_applyingTheme) {
            m_applyingTheme = true;
            applyTheme();
            m_applyingTheme = false;
        }
    }
    QWidget::changeEvent(event);
}

void ViewSwitcher::applyTheme()
{
    const bool dark = DDE25::themeType() == 2;

    // 浅色：白底 + 半透明黑边框 + 黑字；深色：深色半透明底 + 浅色边框 + 白字，
    // 观感与标题栏按钮、gxde-file-manager 标题栏控件保持一致。
    const QString containerBg = dark ? QStringLiteral("rgba(255, 255, 255, 0.08)")
                                    : QStringLiteral("white");
    const QString containerBorder = dark ? QStringLiteral("rgba(255, 255, 255, 0.12)")
                                        : QStringLiteral("rgba(0, 0, 0, 0.1)");
    const QString textColor = dark ? QStringLiteral("rgba(255, 255, 255, 0.9)")
                                   : QStringLiteral("#000000");
    const QString hoverBg = dark ? QStringLiteral("rgba(255, 255, 255, 0.1)")
                                 : QStringLiteral("rgba(0, 0, 0, 0.05)");

    setStyleSheet(
        QString("QWidget#ViewSwitcher {"
                "  background-color: %1;"
                "  border: 1px solid %2;"
                "  border-radius: 4px;"
                "}"
                "QWidget#ViewSwitcher QPushButton {"
                "  background-color: transparent;"
                "  border: none;"
                "  color: %3;"
                "  padding: 0px 6px;"
                "  border-radius: 3px;"
                "}"
                "QWidget#ViewSwitcher QPushButton:hover {"
                "  background-color: %4;"
                "}"
                "QWidget#ViewSwitcher QPushButton:checked {"
                "  background-color: #2ca7f8;"
                "  color: white;"
                "}")
            .arg(containerBg, containerBorder, textColor, hoverBg));
}
