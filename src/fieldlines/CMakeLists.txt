cmake_minimum_required(VERSION 3.5)
project(screensaver.rsxs.fieldlines)

message(STATUS "--------------------------------------------------------------------------------")
message(STATUS "Create \"${CMAKE_PROJECT_NAME}\"")

unset(ADDON_DEPENDS)
unset(USED_SOURCES)
set_property(DIRECTORY PROPERTY COMPILE_DEFINITIONS ${BASE_DEFINITIONS})

set(FIELDLINES_SOURCES ${CMAKE_CURRENT_LIST_DIR}/main.cpp
                       ${CMAKE_CURRENT_LIST_DIR}/ion.cpp)
set(FIELDLINES_HEADERS ${CMAKE_CURRENT_LIST_DIR}/main.h
                       ${CMAKE_CURRENT_LIST_DIR}/ion.h)

build_addon(screensaver.rsxs.fieldlines FIELDLINES DEPLIBS)
