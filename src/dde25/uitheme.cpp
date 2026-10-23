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
 * 本文件为 GXDE 日历原创：全局 UI 约定的唯一事实来源。
 * ----------------------------------------------------------------------------
 */

#include "uitheme.h"

#include <QApplication>
#include <QPalette>

#include "dde25common.h"

namespace DDE25 {

namespace {

// 三个主题色的字面值，整个应用只在这里出现一次。
// 主色沿用界面上已有的蓝，警示色沿用周末/节假日的橙红，
// 中性色沿用正文的深蓝灰（深色主题下换成浅蓝灰）。
const char kAccentHex[] = "#2CA7F8";
const char kAlertHex[] = "#FF5B38";
const char kNeutralLightHex[] = "#414D68";
const char kNeutralDarkHex[] = "#C0C6D4";

QColor withAlpha(const QColor &base, qreal alpha)
{
    QColor color = base;
    color.setAlphaF(alpha);
    return color;
}

} // namespace

QColor accentColor()
{
    return QColor(kAccentHex);
}

QColor alertColor()
{
    return QColor(kAlertHex);
}

QColor neutralColor()
{
    return QColor(themeType() == 2 ? kNeutralDarkHex : kNeutralLightHex);
}

QColor textColor(TextRole role)
{
    switch (role) {
    case TextSecondary:
        return withAlpha(neutralColor(), 0.6);
    case TextTertiary:
        return withAlpha(neutralColor(), 0.4);
    default:
        return neutralColor();
    }
}

QColor onAccentColor()
{
    // 主色是固定的中蓝，深浅主题下都配白字。用调色板的 HighlightedText 会在
    // 系统高亮色被改成浅色时翻车，这里直接钉死。
    return QColor(Qt::white);
}

QColor borderColor()
{
    return withAlpha(neutralColor(), 0.1);
}

QColor hoverColor()
{
    return withAlpha(neutralColor(), 0.06);
}

QColor pressedColor()
{
    return withAlpha(accentColor(), 0.2);
}

QColor hoverAccentColor()
{
    return withAlpha(accentColor(), 0.85);
}

QColor pressedAccentColor()
{
    return accentColor().darker(120);
}

QColor panelColor()
{
    return qApp->palette().color(QPalette::Base);
}

QColor windowColor()
{
    return qApp->palette().color(QPalette::Window);
}

QString css(const QColor &color)
{
    return QStringLiteral("rgba(%1, %2, %3, %4)")
        .arg(color.red())
        .arg(color.green())
        .arg(color.blue())
        .arg(color.alphaF());
}

QString toolButtonStyleSheet(const QString &objectName)
{
    const QString selector = objectName.isEmpty()
        ? QStringLiteral("QPushButton")
        : QStringLiteral("QPushButton#%1").arg(objectName);

    return QStringLiteral(
        "%1 {"
        "  background-color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: %4px;"
        "}"
        "%1:hover {"
        "  background-color: %5;"
        "  border: 1px solid %6;"
        "}"
        "%1:pressed {"
        "  background-color: %7;"
        "  border: 1px solid %8;"
        "}")
        .arg(selector,
             css(panelColor()),
             css(borderColor()),
             QString::number(Metric::RadiusControl),
             css(withAlpha(accentColor(), 0.12)),
             css(withAlpha(accentColor(), 0.35)),
             css(pressedColor()),
             css(accentColor()));
}

QString segmentedStyleSheet(const QString &objectName, const QString &containerClass)
{
    const QString selector = QStringLiteral("%1#%2").arg(containerClass, objectName);

    return QStringLiteral(
        "%1 {"
        "  background-color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: %4px;"
        "}"
        "%1 QPushButton {"
        "  background-color: transparent;"
        "  border: none;"
        "  color: %5;"
        "  padding: 0px 6px;"
        "  border-radius: %4px;"
        "}"
        "%1 QPushButton:hover {"
        "  background-color: %6;"
        "}"
        "%1 QPushButton:checked {"
        "  background-color: %7;"
        "  color: %8;"
        "}")
        .arg(selector,
             css(panelColor()),
             css(borderColor()),
             QString::number(Metric::RadiusControl),
             css(textColor()),
             css(hoverColor()),
             css(accentColor()),
             css(onAccentColor()));
}

} // namespace DDE25
