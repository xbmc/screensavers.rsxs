project(screensaver.rsxs.biof)

cmake_minimum_required(VERSION 3.0)

enable_language(CXX)

message(STATUS "--------------------------------------------------------------------------------")
message(STATUS "Create \"${CMAKE_PROJECT_NAME}\"")

unset(ADDON_DEPENDS)
unset(USED_SOURCES)
set_property(DIRECTORY PROPERTY COMPILE_DEFINITIONS ${BASE_DEFINITIONS})

set(BIOF_SOURCES ${CMAKE_CURRENT_LIST_DIR}/main.cpp)
set(BIOF_HEADERS ${CMAKE_CURRENT_LIST_DIR}/main.h)

build_addon(screensaver.rsxs.biof BIOF DEPLIBS)
