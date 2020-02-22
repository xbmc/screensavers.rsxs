cmake_minimum_required(VERSION 3.5)
project(screensaver.rsxs.euphoria)

message(STATUS "--------------------------------------------------------------------------------")
message(STATUS "Create \"${CMAKE_PROJECT_NAME}\"")

unset(ADDON_DEPENDS)
unset(USED_SOURCES)
set_property(DIRECTORY PROPERTY COMPILE_DEFINITIONS ${BASE_DEFINITIONS})

set(EUPHORIA_SOURCES ${CMAKE_CURRENT_LIST_DIR}/main.cpp)
set(EUPHORIA_HEADERS ${CMAKE_CURRENT_LIST_DIR}/main.h
                     ${CMAKE_CURRENT_LIST_DIR}/texture.h)

build_addon(screensaver.rsxs.euphoria EUPHORIA DEPLIBS)
