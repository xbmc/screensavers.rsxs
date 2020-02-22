cmake_minimum_required(VERSION 3.5)
project(screensaver.rsxs.busyspheres)

message(STATUS "--------------------------------------------------------------------------------")
message(STATUS "Create \"${CMAKE_PROJECT_NAME}\"")

unset(ADDON_DEPENDS)
unset(USED_SOURCES)
set_property(DIRECTORY PROPERTY COMPILE_DEFINITIONS ${BASE_DEFINITIONS})

set(BUSYSPHERES_SOURCES ${CMAKE_CURRENT_LIST_DIR}/main.cpp)
set(BUSYSPHERES_HEADERS ${CMAKE_CURRENT_LIST_DIR}/main.h)

build_addon(screensaver.rsxs.busyspheres BUSYSPHERES DEPLIBS)
