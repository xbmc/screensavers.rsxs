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
ExternalProject_ADD(rsxs SOURCE_DIR ${PROJECT_SOURCE_DIR}/lib/rsxs-0.9/
                    UPDATE_COMMAND autoreconf -vif configure.ac WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/rsxs-0.9/
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

addon_version(screensaver.rsxs.solarwinds SOLARWINDS)
build_addon(screensaver.rsxs.solarwinds SOLARWINDS_SOURCES DEPLIBS SOLARWINDS_VERSION)
SET_TARGET_PROPERTIES(screensaver.rsxs.solarwinds PROPERTIES
                      COMPILE_FLAGS -DPKGDATADIR="\\"/usr/share/xbmc/addons/screensaver.rsxs.solarwinds\\"")
install_addon(screensaver.rsxs.solarwinds ${SOLARWINDS_VERSION})

addon_version(screensaver.rsxs.plasma PLASMA)
build_addon(screensaver.rsxs.plasma PLASMA_SOURCES DEPLIBS PLASMA_VERSION)
SET_TARGET_PROPERTIES(screensaver.rsxs.plasma PROPERTIES
                      COMPILE_FLAGS -DPKGDATADIR="\\"/usr/share/xbmc/addons/screensaver.rsxs.plasma\\"")
install_addon(screensaver.rsxs.plasma ${PLASMA_VERSION})

addon_version(screensaver.rsxs.euphoria EUPHORIA)
build_addon(screensaver.rsxs.euphoria EUPHORIA_SOURCES DEPLIBS EUPHORIA_VERSION)
SET_TARGET_PROPERTIES(screensaver.rsxs.euphoria PROPERTIES
                      COMPILE_FLAGS -DPKGDATADIR="\\"/usr/share/xbmc/addons/screensaver.rsxs.euphoria\\"")
install_addon(screensaver.rsxs.euphoria ${EUPHORIA_VERSION})

