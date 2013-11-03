PROJECT(visualizations.rsxs)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ENABLE_LANGUAGE(CXX)

FIND_PACKAGE(xbmc REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)

INCLUDE_DIRECTORIES(${OpenGL_INCLUDE_DIR}
                    ${XBMC_INCLUDE_DIR}
                    ${CMAKE_BINARY_DIR}/rsxs-prefix/src/rsxs-build
                    ${PROJECT_SOURCE_DIR}/lib/rsxs-0.9/src
                    ${PROJECT_SOURCE_DIR}/lib/rsxs-0.9/src/euphoria
                    ${PROJECT_SOURCE_DIR}/lib/rsxs-0.9/src/plasma
                    ${PROJECT_SOURCE_DIR}/lib/rsxs-0.9/src/solarwinds)
                        
INCLUDE(ExternalProject)
set(update_command "")
if(BOOTSTRAP_IN_TREE)
  set(update_command UPDATE_COMMAND autoreconf -vif WORKING_DIRECTORY <SOURCE_DIR>)
endif()
ExternalProject_ADD(rsxs SOURCE_DIR ${PROJECT_SOURCE_DIR}/lib/rsxs-0.9/
                    "${update_command}"
                    CONFIGURE_COMMAND ${PROJECT_SOURCE_DIR}/lib/rsxs-0.9/configure
                                      --prefix=<INSTALL_DIR>
                                      --without-xscreensaver
                                      --disable-sound
                                      --disable-cyclone
                                      --disable-fieldlines
                                      --disable-flocks
                                      --disable-flux
                                      --disable-helios
                                      --disable-hyperspace
                                      --disable-lattice
                                      --disable-skyrocket "CFLAGS=-fPIC -I${XBMC_INCLUDE_DIR}" "CXXFLAGS=-fPIC -I${XBMC_INCLUDE_DIR}"
                    BUILD_COMMAND ${MAKE})

ADD_DEFINITIONS(-DHAVE_CONFIG_H)

SET(SOLARWINDS_SOURCES lib/rsxs-0.9/src/color.cc
                       lib/rsxs-0.9/src/common.cc
                       lib/rsxs-0.9/src/solarwinds/solarwinds.cc
                       lib/rsxs-0.9/src/solarwinds/wind.cc)

SET(PLASMA_SOURCES lib/rsxs-0.9/src/color.cc
                   lib/rsxs-0.9/src/common.cc
                   lib/rsxs-0.9/src/plasma/plasma.cc)

SET(EUPHORIA_SOURCES lib/rsxs-0.9/src/color.cc
                     lib/rsxs-0.9/src/common.cc
                     lib/rsxs-0.9/src/euphoria/euphoria.cc
                     lib/rsxs-0.9/src/euphoria/wisp.cc)

SET(DEPLIBS ${OPENGL_LIBRARIES}
            ${GLEW_LIBRARIES}
            ${CMAKE_BINARY_DIR}/rsxs-prefix/src/rsxs-build/lib/libmisc.a)

build_addon(screensaver.rsxs.solarwinds SOLARWINDS DEPLIBS)
SET_TARGET_PROPERTIES(screensaver.rsxs.solarwinds PROPERTIES
                      COMPILE_FLAGS -DPKGDATADIR="\\"/usr/share/xbmc/addons/screensaver.rsxs.solarwinds\\"")
add_dependencies(screensaver.rsxs.solarwinds rsxs)

build_addon(screensaver.rsxs.plasma PLASMA DEPLIBS)
SET_TARGET_PROPERTIES(screensaver.rsxs.plasma PROPERTIES
                      COMPILE_FLAGS -DPKGDATADIR="\\"/usr/share/xbmc/addons/screensaver.rsxs.plasma\\"")
add_dependencies(screensaver.rsxs.plasma rsxs)

build_addon(screensaver.rsxs.euphoria EUPHORIA DEPLIBS)
SET_TARGET_PROPERTIES(screensaver.rsxs.euphoria PROPERTIES
                      COMPILE_FLAGS -DPKGDATADIR="\\"/usr/share/xbmc/addons/screensaver.rsxs.euphoria\\"")
add_dependencies(screensaver.rsxs.euphoria rsxs)

include(CPack)
