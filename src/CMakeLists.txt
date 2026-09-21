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
find_package(Dtk2widget REQUIRED)
find_package(Dtk6Core REQUIRED)

add_executable(${BIN_NAME} ${SRCS} ${QRC})
target_include_directories(${BIN_NAME} PUBLIC
    ${Dtk2widget_INCLUDE_DIRS}
    /usr/include/dtk6/DCore
    /usr/include/dtk6/DLog
    ${Qt6Gui_PRIVATE_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${Dtk2widget_LIBRARIES}
    Dtk6::Core
    ${Qt6Widgets_LIBRARIES}
    ${Qt6Network_LIBRARIES}
    ${Qt6Concurrent_LIBRARIES}
    ${Qt6DBus_LIBRARIES}
)

# bin
install(TARGETS ${BIN_NAME} DESTINATION bin)
