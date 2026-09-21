# KCalendarCore 快照，vendor 自 dde-calendar 的 3rdparty/kcalendarcore，
# 用于 ICS 的解析/序列化（VEVENT、RRULE、VALARM），不自己手写 ICS 解析器。
#
# 相对上游做了三处裁剪，都是为了让它在 gxde-calendar 里当静态库用：
#   1. 上游写的是 find_package(Qt${QT_VERSION_MAJOR} ...)，依赖父工程先 find_package 过
#      Qt 才有 QT_VERSION_MAJOR。这里直接写 Qt6，子目录不再单独探测。
#   2. 上游把 -pie 塞进了 CMAKE_CXX_FLAGS。那是链接期参数，对静态库没有意义，
#      位置无关由下面的 POSITION_INDEPENDENT_CODE 负责。
#   3. 上游链了 Qt::DBus 和 -licalss/-licalvcal。这套源码没有任何 QDBus 引用，
#      libical.pc 的 Libs 里已经含 -lical -licalss -licalvcal，不必再单列。

cmake_minimum_required(VERSION 3.7)
project(kcalendarcore)

# Qt6 头文件要求 C++17；顶层工程设的是 14，这里覆盖掉
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(PkgConfig REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui)
pkg_check_modules(3rd_lib REQUIRED libical)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src ${3rd_lib_INCLUDE_DIRS})
aux_source_directory(src CALENDARCORE_SRCS)

add_library(${PROJECT_NAME} STATIC ${CALENDARCORE_SRCS})
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    ${3rd_lib_LIBRARIES}
)
