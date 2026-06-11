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
./weekindicator.cpp
./spinner.cpp
./infoview.cpp
./calendardbus.cpp
./calendarview.cpp
./main.cpp
./dbuscalendar_adaptor.cpp
./calendarwindow.cpp
./holidayapi.cpp
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
