project(screensaver.rsxs.drempels)

cmake_minimum_required(VERSION 3.3)

enable_language(CXX)

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
                     ${CMAKE_CURRENT_LIST_DIR}/TexMgr.h)

set(ADDON_DEPENDS ImageMagick)
include_directories(${ImageMagick_INCLUDE_DIRS})
set(DREMPELS_DEPLIBS ${DEPLIBS}
                     ${ImageMagick_LIBRARIES}
                     ${JPEG_LIBRARIES}
                     ${PNG_LIBRARIES}
                     ${TIFF_LIBRARIES}
                     ${LTDL_LIBRARIES})

build_addon(screensaver.rsxs.drempels DREMPELS DREMPELS_DEPLIBS)
if(NOT ImageMagick_FOUND)
  # Force build of own if ImageMagick is not present
  add_dependencies(screensaver.rsxs.drempels ImageMagick)
endif()
