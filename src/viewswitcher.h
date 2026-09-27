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

#ifndef SRC_VIEWSWITCHER_H_
#define SRC_VIEWSWITCHER_H_

#include <QWidget>

#include "dde25/dde25common.h"

class QButtonGroup;
class QPushButton;

class ViewSwitcher : public QWidget {
    Q_OBJECT
public:
    explicit ViewSwitcher(QWidget *parent = nullptr);

    void setLabels(const QStringList &labels);
    void setCurrentIndex(int index);
    int currentIndex() const;

signals:
    void currentChanged(int index);

protected:
    void changeEvent(QEvent *event) override;

private:
    // 按系统深浅主题刷新容器背景、边框与各分段按钮的文字/悬停/选中色
    void applyTheme();

    QButtonGroup *m_group = nullptr;
    QList<QPushButton *> m_buttons;
    // setStyleSheet() 会触发 Qt 内部重设调色板并再次分发 PaletteChange，
    // 若不加保护会在 changeEvent -> applyTheme -> setStyleSheet 间无限递归。
    bool m_applyingTheme = false;
};

#endif  // SRC_VIEWSWITCHER_H_
