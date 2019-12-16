cmake_minimum_required(VERSION 3.5)
project(screensaver.rsxs.skyrocket)

message(STATUS "--------------------------------------------------------------------------------")
message(STATUS "Create \"${CMAKE_PROJECT_NAME}\"")

unset(ADDON_DEPENDS)
unset(USED_SOURCES)
set_property(DIRECTORY PROPERTY COMPILE_DEFINITIONS ${BASE_DEFINITIONS})

set(SKYROCKET_SOURCES ${CMAKE_CURRENT_LIST_DIR}/main.cpp
                      ${CMAKE_CURRENT_LIST_DIR}/flare.cpp
                      ${CMAKE_CURRENT_LIST_DIR}/particle.cpp
                      ${CMAKE_CURRENT_LIST_DIR}/shockwave.cpp
                      ${CMAKE_CURRENT_LIST_DIR}/smoke.cpp
                      ${CMAKE_CURRENT_LIST_DIR}/soundEngine.cpp
                      ${CMAKE_CURRENT_LIST_DIR}/world.cpp)

set(SKYROCKET_HEADERS ${CMAKE_CURRENT_LIST_DIR}/main.h
                      ${CMAKE_CURRENT_LIST_DIR}/boomsound.h
                      ${CMAKE_CURRENT_LIST_DIR}/cloudtex.h
                      ${CMAKE_CURRENT_LIST_DIR}/earthtex.h
                      ${CMAKE_CURRENT_LIST_DIR}/flare.h
                      ${CMAKE_CURRENT_LIST_DIR}/launchsound.h
                      ${CMAKE_CURRENT_LIST_DIR}/moontex.h
                      ${CMAKE_CURRENT_LIST_DIR}/nukesound.h
                      ${CMAKE_CURRENT_LIST_DIR}/particle.h
                      ${CMAKE_CURRENT_LIST_DIR}/poppersound.h
                      ${CMAKE_CURRENT_LIST_DIR}/shockwave.h
                      ${CMAKE_CURRENT_LIST_DIR}/smoke.h
                      ${CMAKE_CURRENT_LIST_DIR}/smoketex.h
                      ${CMAKE_CURRENT_LIST_DIR}/soundEngine.h
                      ${CMAKE_CURRENT_LIST_DIR}/sucksound.h
                      ${CMAKE_CURRENT_LIST_DIR}/whistlesound.h
                      ${CMAKE_CURRENT_LIST_DIR}/world.h)

build_addon(screensaver.rsxs.skyrocket SKYROCKET DEPLIBS)
