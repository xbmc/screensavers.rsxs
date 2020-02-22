cmake_minimum_required(VERSION 3.5)
project(screensaver.rsxs.sundancer2)

message(STATUS "--------------------------------------------------------------------------------")
message(STATUS "Create \"${CMAKE_PROJECT_NAME}\"")

unset(ADDON_DEPENDS)
unset(USED_SOURCES)
set_property(DIRECTORY PROPERTY COMPILE_DEFINITIONS ${BASE_DEFINITIONS})

set(SUNDANCER2_SOURCES ${CMAKE_CURRENT_LIST_DIR}/main.cpp)
set(SUNDANCER2_HEADERS ${CMAKE_CURRENT_LIST_DIR}/main.h)

build_addon(screensaver.rsxs.sundancer2 SUNDANCER2 DEPLIBS)
