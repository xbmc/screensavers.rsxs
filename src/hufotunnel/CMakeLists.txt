project(screensaver.rsxs.hufotunnel)

cmake_minimum_required(VERSION 3.0)

enable_language(CXX)

message(STATUS "--------------------------------------------------------------------------------")
message(STATUS "Create \"${CMAKE_PROJECT_NAME}\"")

find_package(BZip2 REQUIRED)

unset(ADDON_DEPENDS)
unset(USED_SOURCES)
set_property(DIRECTORY PROPERTY COMPILE_DEFINITIONS ${BASE_DEFINITIONS})

set(HUFOTUNNEL_SOURCES ${CMAKE_CURRENT_LIST_DIR}/main.cpp
                       ${CMAKE_CURRENT_LIST_DIR}/hufo_tunnel_textures.cpp)
set(HUFOTUNNEL_HEADERS ${CMAKE_CURRENT_LIST_DIR}/main.h
                       ${CMAKE_CURRENT_LIST_DIR}/hufo_tunnel_textures.h)

set(HUFOTUNNEL_DEPLIBS ${DEPLIBS} ${BZIP2_LIBRARIES})

build_addon(screensaver.rsxs.hufotunnel HUFOTUNNEL HUFOTUNNEL_DEPLIBS)
