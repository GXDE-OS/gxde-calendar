cmake_minimum_required(VERSION 3.7)

project(dde-calendar)

# set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-g -Wall")

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

    # generate qm
    execute_process(COMMAND bash "translate_generation.sh"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    # generate desktop
    execute_process(COMMAND bash "translate_desktop.sh"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif ()

add_subdirectory("src")

#qm files
file(GLOB QM_FILES "translations/*.qm")
install(FILES ${QM_FILES} DESTINATION share/dde-calendar/translations)

#desktop
install(FILES dde-calendar.desktop DESTINATION share/applications)

#service
install(FILES com.deepin.Calendar.service DESTINATION share/dbus-1/services)

#icon
install(FILES src/resources/icon/dde-calendar.svg DESTINATION share/icons/hicolor/scalable/apps/)
