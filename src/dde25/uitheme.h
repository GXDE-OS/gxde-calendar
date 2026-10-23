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

#ifndef UITHEME_H
#define UITHEME_H

#include <QColor>
#include <QString>

namespace DDE25 {

// ============================================================================
// 三条硬性约束，全部落在这个文件里，别处不许就地写颜色/尺寸字面值：
//
//   1. 主题色只有三个：accentColor() / alertColor() / neutralColor()。
//      其余颜色一律是这三个色（或系统调色板）的透明度派生，不得出现第四种色相。
//      黑白（Qt::white / Qt::black）是无彩色，不占主题色名额。
//   2. 同一元素只取一个尺寸：圆角、图标、控件高、行高等都从 DDE25::Metric 取，
//      同一个元素上不允许出现两个圆角 / 两个字号 / 两个图标尺寸。
//   3. 任何功能从窗口可见状态起 3 次点击内可达。
// ============================================================================

// 主题色 1/3：主色。选中态、今日、工具栏高亮、主按钮、日程类型默认色
QColor accentColor();

// 主题色 2/3：警示色。周末 / 节假日 / 删除 / 失败提示
QColor alertColor();

// 主题色 3/3：中性色（正文基准色），随深浅主题切换
QColor neutralColor();

// 正文的三个层级，全部是中性色的透明度派生，不引入新色相
enum TextRole {
    TextPrimary,    // 正文：中性色 100%
    TextSecondary,  // 次级（时间、说明）：中性色 60%
    TextTertiary,   // 弱 / 禁用 / 非当前月：中性色 40%
};

QColor textColor(TextRole role = TextPrimary);

// 画在主色底上的文字（选中圆底里的日期、checked 分段上的文字）
QColor onAccentColor();

// 1px 分隔线 / 控件描边：中性色 10%
QColor borderColor();
// 悬浮底色：中性色 6%
QColor hoverColor();
// 按下底色：主色 20%
QColor pressedColor();
// 链接/文字态的主色变体：hover 浅一档、pressed 深一档，仍是主色本身
QColor hoverAccentColor();
QColor pressedAccentColor();

// 面板（侧栏、卡片、弹窗内容）底色与窗口底色：取自系统调色板，不是主题色
QColor panelColor();
QColor windowColor();

// 供 setStyleSheet 使用的 rgba(r, g, b, a) 串
QString css(const QColor &color);

// ---------------------------------------------------------------------------
// 尺寸尺度：同一个元素只允许取其中一个值
// ---------------------------------------------------------------------------
namespace Metric {
// 图标：整个应用只有这一个图标尺寸
constexpr int Icon = 16;
// 标题栏 / 列表头上的方形图标按钮
constexpr int ToolButton = 24;
// 视图内的翻页按钮与 Today 按钮的高度
constexpr int NavButton = 36;
// 圆角：按钮、分段控件、输入框
constexpr int RadiusControl = 4;
// 圆角：列表项、卡片
constexpr int RadiusItem = 6;
// 标题栏控件统一高度（Y/M/W/D 与两侧图标按钮同高）
constexpr int ToolbarHeight = 24;
// 日程块 / 日程行的左侧类型色条宽度
constexpr int ColorBar = 4;
// 侧栏日程行的固定高度与行内两条文字的行高
constexpr int ScheduleRowHeight = 44;
constexpr int ScheduleRowLine = 18;
} // namespace Metric

// 标题栏 / 列表头上的方形图标按钮（侧栏折叠、新建日程、侧栏列表的「+」）
// 共用这一份样式：颜色全部从上面的主题色派生，圆角统一为 Metric::RadiusControl。
// objectName 为空时作用于所有 QPushButton。
QString toolButtonStyleSheet(const QString &objectName = QString());

// 分段控件（Y/M/W/D、年视图的 ‹ Today ›）：容器与分段共用同一个圆角，
// 避免出现「外框 4px、内块 3px」这种同一元素两种尺寸。
// containerClass 是容器自身的类名（QWidget / QFrame），Qt 样式表的类型选择器
// 不匹配子类，所以这里要按实际类型传。
QString segmentedStyleSheet(const QString &objectName,
                            const QString &containerClass = QStringLiteral("QWidget"));

} // namespace DDE25

#endif // UITHEME_H
