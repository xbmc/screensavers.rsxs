project(screensaver.rsxs.hyperspace)

cmake_minimum_required(VERSION 3.0)

enable_language(CXX)

message(STATUS "--------------------------------------------------------------------------------")
message(STATUS "Create \"${CMAKE_PROJECT_NAME}\"")

unset(ADDON_DEPENDS)
unset(USED_SOURCES)
set_property(DIRECTORY PROPERTY COMPILE_DEFINITIONS ${BASE_DEFINITIONS})

set(HYPERSPACE_SOURCES ${CMAKE_CURRENT_LIST_DIR}/main.cpp
                       ${CMAKE_CURRENT_LIST_DIR}/causticTextures.cpp
                       ${CMAKE_CURRENT_LIST_DIR}/flare.cpp
                       ${CMAKE_CURRENT_LIST_DIR}/goo.cpp
                       ${CMAKE_CURRENT_LIST_DIR}/mipmap.cpp
                       ${CMAKE_CURRENT_LIST_DIR}/splinePath.cpp
                       ${CMAKE_CURRENT_LIST_DIR}/starBurst.cpp
                       ${CMAKE_CURRENT_LIST_DIR}/stretchedParticle.cpp
                       ${CMAKE_CURRENT_LIST_DIR}/tunnel.cpp
                       ${CMAKE_CURRENT_LIST_DIR}/wavyNormalCubeMaps.cpp)

set(HYPERSPACE_HEADERS ${CMAKE_CURRENT_LIST_DIR}/main.h
                       ${CMAKE_CURRENT_LIST_DIR}/causticTextures.h
                       ${CMAKE_CURRENT_LIST_DIR}/flare.h
                       ${CMAKE_CURRENT_LIST_DIR}/goo.h
                       ${CMAKE_CURRENT_LIST_DIR}/mipmap.h
                       ${CMAKE_CURRENT_LIST_DIR}/nebulamap.h
                       ${CMAKE_CURRENT_LIST_DIR}/splinePath.h
                       ${CMAKE_CURRENT_LIST_DIR}/starBurst.h
                       ${CMAKE_CURRENT_LIST_DIR}/stretchedParticle.h
                       ${CMAKE_CURRENT_LIST_DIR}/tunnel.h
                       ${CMAKE_CURRENT_LIST_DIR}/wavyNormalCubeMaps.h)

build_addon(screensaver.rsxs.hyperspace HYPERSPACE DEPLIBS)
