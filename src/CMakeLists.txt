cmake_minimum_required(VERSION 3.7)

set(BIN_NAME gxde-calendar)

configure_file(environments.h.in environments.h @ONLY)

set(SRCS
./environments.h
./weekindicator.h
./spinner.h
./calendarwindow.h
./infoview.h
./calendardbus.h
./dbuscalendar_adaptor.h
./calendarview.h
./constants.h
./holidayapi.h
./sidebarcalendarwidget.h
./calendarviews.h
./viewswitcher.h
./weekindicator.cpp
./spinner.cpp
./infoview.cpp
./calendardbus.cpp
./calendarview.cpp
./main.cpp
./dbuscalendar_adaptor.cpp
./calendarwindow.cpp
./holidayapi.cpp
./sidebarcalendarwidget.cpp
./calendarviews.cpp
./viewswitcher.cpp
)

# DDE 25 视图：移植自 dde-calendar，仅供 DDE 25 布局使用
list(APPEND SRCS
./dde25/dde25common.h
./dde25/customframe.h
./dde25/cfocusitem.h
./dde25/cscenebackgrounditem.h
./dde25/cmonthdayitem.h
./dde25/cweekdaybackgrounditem.h
./dde25/monthweekview.h
./dde25/monthgraphicsview.h
./dde25/monthdayview.h
./dde25/monthview.h
./dde25/monthwindow.h
./dde25/weekheadview.h
./dde25/weeknumview.h
./dde25/weekgraphicsview.h
./dde25/weekwindow.h
./dde25/schedulebodyview.h
./dde25/cweekwidget.h
./dde25/daymonthview.h
./dde25/daywindow.h
./dde25/dde25common.cpp
./dde25/customframe.cpp
./dde25/cfocusitem.cpp
./dde25/cscenebackgrounditem.cpp
./dde25/cmonthdayitem.cpp
./dde25/cweekdaybackgrounditem.cpp
./dde25/monthweekview.cpp
./dde25/monthgraphicsview.cpp
./dde25/monthdayview.cpp
./dde25/monthview.cpp
./dde25/monthwindow.cpp
./dde25/weekheadview.cpp
./dde25/weeknumview.cpp
./dde25/weekgraphicsview.cpp
./dde25/weekwindow.cpp
./dde25/schedulebodyview.cpp
./dde25/cweekwidget.cpp
./dde25/daymonthview.cpp
./dde25/daywindow.cpp
)

# 日程数据层：本地日历 + ICS（文件导入/导出、URL 订阅），无账户系统。
# 移植自 dde-calendar，见各文件头部注释。
list(APPEND SRCS
./schedule/commondef.h
./schedule/units.h
./schedule/dschedule.h
./schedule/dscheduletype.h
./schedule/dtypecolor.h
./schedule/dschedulequerypar.h
./schedule/ddatabase.h
./schedule/scheduledatabase.h
./schedule/icsmanager.h
./schedule/calendarservice.h
./schedule/units.cpp
./schedule/dschedule.cpp
./schedule/dscheduletype.cpp
./schedule/dtypecolor.cpp
./schedule/dschedulequerypar.cpp
./schedule/ddatabase.cpp
./schedule/scheduledatabase.cpp
./schedule/icsmanager.cpp
./schedule/calendarservice.cpp
./schedule/pinyin/pinyindict.h
./schedule/pinyin/pinyinsearch.h
./schedule/pinyin/pinyindict.cpp
./schedule/pinyin/pinyinsearch.cpp
./schedule/lunarandfestival/lunarandfestival.h
./schedule/lunarandfestival/celestialbodies.h
./schedule/lunarandfestival/method_interface.h
./schedule/lunarandfestival/lunarcalendar.h
./schedule/lunarandfestival/lunardateinfo.h
./schedule/lunarandfestival/celestialbodies.cpp
./schedule/lunarandfestival/method_interface.cpp
./schedule/lunarandfestival/lunarcalendar.cpp
./schedule/lunarandfestival/lunardateinfo.cpp
)

set(QRC
./resources.qrc
)

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt6Widgets REQUIRED)
find_package(Qt6Concurrent REQUIRED)
find_package(Qt6DBus REQUIRED)
find_package(Qt6Network REQUIRED)
find_package(Qt6Gui REQUIRED)
find_package(Qt6Sql REQUIRED)
find_package(Dtk2widget REQUIRED)
find_package(Dtk6Core REQUIRED)

add_executable(${BIN_NAME} ${SRCS} ${QRC})
target_include_directories(${BIN_NAME} PUBLIC
    ${Dtk2widget_INCLUDE_DIRS}
    /usr/include/dtk6/DCore
    /usr/include/dtk6/DLog
    ${Qt6Gui_PRIVATE_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
    # KCalendarCore 的头文件是平铺的（#include "event.h" 这种），
    # 参考实现也是直接 include_directories 它的 src 目录。
    ${PROJECT_SOURCE_DIR}/3rdparty/kcalendarcore/src
    # schedule/ 下的子目录（pinyin、lunarandfestival）按参考实现的写法
    # #include "commondef.h"、#include "pinyin/pinyinsearch.h"，
    # 参考的 include_directories 里有 calendar-common/src，这里对应 schedule/。
    ${CMAKE_CURRENT_SOURCE_DIR}/schedule
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${Dtk2widget_LIBRARIES}
    Dtk6::Core
    ${Qt6Widgets_LIBRARIES}
    ${Qt6Network_LIBRARIES}
    ${Qt6Concurrent_LIBRARIES}
    ${Qt6DBus_LIBRARIES}
    ${Qt6Sql_LIBRARIES}
    kcalendarcore
)

# bin
install(TARGETS ${BIN_NAME} DESTINATION bin)
