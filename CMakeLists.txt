PROJECT(visualizations.rsxs)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
ENABLE_LANGUAGE(CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

# Locate XBMC bindings
FIND_PATH(XBMC_INCLUDE_DIR xbmc/xbmc_scr_dll.h)

FIND_PACKAGE(OpenGL REQUIRED)
INCLUDE_DIRECTORIES(${OpenGL_INCLUDE_DIR}
                    ${XBMC_INCLUDE_DIR}
                    ${CMAKE_BINARY_DIR}/rsxs-prefix/src/rsxs-build
                    ${PROJECT_SOURCE_DIR}/lib/rsxs-0.9/src
                    ${PROJECT_SOURCE_DIR}/lib/rsxs-0.9/src/euphoria
                    ${PROJECT_SOURCE_DIR}/lib/rsxs-0.9/src/plasma
                    ${PROJECT_SOURCE_DIR}/lib/rsxs-0.9/src/solarwinds)
                        
INCLUDE(ExternalProject)
ExternalProject_ADD(rsxs SOURCE_DIR ${PROJECT_SOURCE_DIR}/lib/rsxs-0.9/
                    UPDATE_COMMAND autoreconf -vif configure.ac WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/rsxs-0.9/
                    CONFIGURE_COMMAND ${PROJECT_SOURCE_DIR}/lib/rsxs-0.9/configure
                                      --prefix=<INSTALL_DIR> --without-xscreensaver
                                      --disable-sound
                                      --disable-cyclone
                                      --disable-fieldlines
                                      --disable-flocks
                                      --disable-flux
                                      --disable-helios
                                      --disable-hyperspace
                                      --disable-lattice
                                      --disable-skyrocket CFLAGS=-fPIC CXXFLAGS=-fPIC
                    BUILD_COMMAND ${MAKE})

SET(SOLARWIND_SOURCES lib/rsxs-0.9/src/color.cc
                      lib/rsxs-0.9/src/common.cc
                      lib/rsxs-0.9/src/solarwinds/solarwinds.cc
                      lib/rsxs-0.9/src/solarwinds/wind.cc)
ADD_LIBRARY(solarwinds ${SOLARWIND_SOURCES})
ADD_DEFINITIONS(-DHAVE_CONFIG_H)
SET_TARGET_PROPERTIES(solarwinds PROPERTIES
                      COMPILE_FLAGS -DPKGDATADIR="\\"/usr/share/xbmc/addons/screensaver.rsxs.solarwinds\\"")
ADD_DEPENDENCIES(solarwinds rsxs)
TARGET_LINK_LIBRARIES(solarwinds ${OPENGL_LIBRARIES}
                                 ${GLEW_LIBRARIES}
                                 ${CMAKE_BINARY_DIR}/rsxs-prefix/src/rsxs-build/lib/libmisc.a)
SET_TARGET_PROPERTIES(solarwinds
                      PROPERTIES VERSION 1.0.0
                      SOVERSION 13.0
                      PREFIX "")

INSTALL(TARGETS solarwinds DESTINATION lib/xbmc/addons/screensaver.rsxs.solarwinds)
INSTALL(DIRECTORY screensaver.rsxs.solarwinds DESTINATION share/xbmc/addons)

SET(PLASMA_SOURCES lib/rsxs-0.9/src/color.cc
                   lib/rsxs-0.9/src/common.cc
                   lib/rsxs-0.9/src/plasma/plasma.cc)
ADD_LIBRARY(plasma ${PLASMA_SOURCES})
ADD_DEFINITIONS(-DHAVE_CONFIG_H)
SET_TARGET_PROPERTIES(plasma PROPERTIES
                      COMPILE_FLAGS -DPKGDATADIR="\\"/usr/share/xbmc/addons/screensaver.rsxs.plasma\\"")
ADD_DEPENDENCIES(plasma rsxs)
TARGET_LINK_LIBRARIES(plasma ${OPENGL_LIBRARIES}
                                 ${GLEW_LIBRARIES}
                                 ${CMAKE_BINARY_DIR}/rsxs-prefix/src/rsxs-build/lib/libmisc.a)
SET_TARGET_PROPERTIES(plasma
                      PROPERTIES VERSION 1.0.0
                      SOVERSION 13.0
                      PREFIX "")

INSTALL(TARGETS plasma DESTINATION lib/xbmc/addons/screensaver.rsxs.plasma)
INSTALL(DIRECTORY screensaver.rsxs.plasma DESTINATION share/xbmc/addons)

SET(EUPHORIA_SOURCES lib/rsxs-0.9/src/color.cc
                   lib/rsxs-0.9/src/common.cc
                   lib/rsxs-0.9/src/euphoria/euphoria.cc
                   lib/rsxs-0.9/src/euphoria/wisp.cc)
ADD_LIBRARY(euphoria ${EUPHORIA_SOURCES})
ADD_DEFINITIONS(-DHAVE_CONFIG_H)
SET_TARGET_PROPERTIES(euphoria PROPERTIES
                      COMPILE_FLAGS -DPKGDATADIR="\\"/usr/share/xbmc/addons/screensaver.rsxs.euphoria\\"")
ADD_DEPENDENCIES(euphoria rsxs)
TARGET_LINK_LIBRARIES(euphoria ${OPENGL_LIBRARIES}
                                ${GLEW_LIBRARIES}
                                ${CMAKE_BINARY_DIR}/rsxs-prefix/src/rsxs-build/lib/libmisc.a)
SET_TARGET_PROPERTIES(euphoria
                      PROPERTIES VERSION 1.0.0
                      SOVERSION 13.0
                      PREFIX "")

INSTALL(TARGETS euphoria DESTINATION lib/xbmc/addons/screensaver.rsxs.euphoria)
INSTALL(DIRECTORY screensaver.rsxs.euphoria DESTINATION share/xbmc/addons)
