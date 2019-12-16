cmake_minimum_required(VERSION 3.5)
project(screensaver.rsxs.drempels)

message(STATUS "--------------------------------------------------------------------------------")
message(STATUS "Create \"${CMAKE_PROJECT_NAME}\"")

unset(ADDON_DEPENDS)
unset(USED_SOURCES)
set_property(DIRECTORY PROPERTY COMPILE_DEFINITIONS ${BASE_DEFINITIONS})

set(DREMPELS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/main.cpp
                     ${CMAKE_CURRENT_LIST_DIR}/gpoly.cpp
                     ${CMAKE_CURRENT_LIST_DIR}/noise1234.c
                     ${CMAKE_CURRENT_LIST_DIR}/TexMgr.cpp)

set(DREMPELS_HEADERS ${CMAKE_CURRENT_LIST_DIR}/main.h
                     ${CMAKE_CURRENT_LIST_DIR}/gpoly.h
                     ${CMAKE_CURRENT_LIST_DIR}/noise1234.h
                     ${CMAKE_CURRENT_LIST_DIR}/stb_image.h
                     ${CMAKE_CURRENT_LIST_DIR}/stb_image_resize.h
                     ${CMAKE_CURRENT_LIST_DIR}/TexMgr.h)

build_addon(screensaver.rsxs.drempels DREMPELS DEPLIBS)
