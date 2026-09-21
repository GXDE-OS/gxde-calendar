/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     kirigaya <kirigaya@mkacg.com>
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

#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace DDECalendar {
    static const int HeaderItemWidth = 80;
    static const int HeaderItemHeight = 40;

    static const int CellWidth = 106;
    static const int CellHeight = 70;

    static const int CellHighlightWidth = 80;
    static const int CellHighlightHeight = 70;

    // 以下常量由 dde-calendar 移植而来，供 DDE 25 视图使用。
    static const int QueryEarliestYear = 1900;
    static const int QueryLatestYear = 2100;

    static const int FontSizeEleven = 11;
    static const int FontSizeTwelve = 12;
    static const int FontSizeFourteen = 14;
    static const int FontSizeSixteen = 16;
    static const int FontSizeTwenty = 20;
    static const int FontSizeTwentyfour = 24;
    static const int FontSizeTen = 10;
    static const int FontSizeOneHundred = 100;
}

namespace DDEMonthCalendar {
    static const int ItemSizeOfMonthDay = 42;

    static const int AFewDaysOfWeek = 7;
    static const int LinesNumOfMonth = 6;

    static const int MonthNumOfYear = 12;

    static const int MHeaderItemWidth = 60;
    static const int MHeaderItemHeight = 33;
    static const int MEventsItemHeight = 36;

    static const int MCellWidth = 120;
    static const int MCellHeight = 74;

    static const int MCellHighlightWidth = 120;
    static const int MCellHighlightHeight = 74;

    static const int MWeekCellWidth = 120;
    static const int MWeekCellHeight = 54;

    static const int MDayCellWidth = 36;
    static const int MDayCellHeight = 36;

    static const int M_YTopHeight = 66;
    static const int M_YLabelHeight = 36;
    static const int M_YLunaLabelWindth = 66;
    static const int M_YLunaLabelHeight = 20;

    static const int MTodayWindth = 100;
    static const int MTodayHeight = 36;
}

namespace DDEWeekCalendar {
    static const int FirstDayOfWeekend = 6;
    static const int AFewDaysofWeek = 7;

    static const int NumWeeksDisplayed = 10;

    static const int WCellHeaderItemWidth = 109;
    static const int WCellHeaderItemHeight = 54;

    static const int WMCellHeaderWidth = 80;

    static const int WWeekCellWidth = 36;
    static const int WWeekCellHeight = 36;

    static const int W_YLabelHeight = 36;
    static const int W_YLunatLabelWindth = 66;
    static const int W_YLunatLabelHeight = 20;

    static const int WTodayWindth = 100;
    static const int WTodayHeight = 36;
}

namespace DDEDayCalendar {
    static const int PainterCellNum = 42;

    static const int DCellWidth = 35;
    static const int DCellHeight = 28;

    static const int DCellHighlightWidth = 33;
    static const int DCellHighlightHeight = 26;
    static const int D_MLabelHeight = 36;
    static const int D_YLabelHeight = 36;
    static const int DDLabelHeight = 117;
    static const int DWLabelHeight = 22;
    // 农历干支信息行；参考实现里其下方还有宜/忌两块的尺寸常量
    // （DHuangLiLabel{Height,MaxHeight,Width}），本项目未移植宜/忌，故未引入。
    static const int DHuangLiInfoLabelHeight = 17;
}

enum Week {
    Monday    = 6,
    Tuesday   = 5,
    Wednesday = 4,
    Thursday  = 3,
    Friday    = 2,
    Saturday  = 1,
    Sunday    = 0,
};

#endif // CONSTANTS_H
