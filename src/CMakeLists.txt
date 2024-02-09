cmake_minimum_required(VERSION 3.7)

set(BIN_NAME dde-calendar)

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
./weekindicator.cpp
./spinner.cpp
./infoview.cpp
./calendardbus.cpp
./calendarview.cpp
./main.cpp
./dbuscalendar_adaptor.cpp
./calendarwindow.cpp
)

set(QRC
./resources.qrc
)

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(DtkWidget REQUIRED)

add_executable(${BIN_NAME} ${SRCS} ${QRC})
target_include_directories(${BIN_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${DtkWidget_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
)

# bin
install(TARGETS ${BIN_NAME} DESTINATION bin)
