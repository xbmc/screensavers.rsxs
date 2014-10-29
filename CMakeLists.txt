project(visualizations.rsxs)

cmake_minimum_required(VERSION 2.6)

enable_language(CXX)

find_package(kodi REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(${OpenGL_INCLUDE_DIR}
                    ${KODI_INCLUDE_DIR}
                    ${CMAKE_BINARY_DIR}/rsxs-prefix/src/rsxs-build
                    ${PROJECT_SOURCE_DIR}/lib/rsxs-0.9/src
                    ${PROJECT_SOURCE_DIR}/lib/rsxs-0.9/src/euphoria
                    ${PROJECT_SOURCE_DIR}/lib/rsxs-0.9/src/plasma
                    ${PROJECT_SOURCE_DIR}/lib/rsxs-0.9/src/solarwinds)
                        
include(ExternalProject)
set(update_command "")
if(BOOTSTRAP_IN_TREE OR NOT DEFINED BOOTSTRAP_IN_TREE)
  set(update_command UPDATE_COMMAND autoreconf -vif WORKING_DIRECTORY <SOURCE_DIR>)
endif()
externalproject_add(rsxs SOURCE_DIR ${PROJECT_SOURCE_DIR}/lib/rsxs-0.9/
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
                                      --disable-skyrocket "CFLAGS=-fPIC -I${KODI_INCLUDE_DIR}" "CXXFLAGS=-fPIC -I${KODI_INCLUDE_DIR}"
                    BUILD_COMMAND ${MAKE}
                    "${update_command}")

add_definitions(-DHAVE_CONFIG_H)

set(SOLARWINDS_SOURCES lib/rsxs-0.9/src/color.cc
                       lib/rsxs-0.9/src/common.cc
                       lib/rsxs-0.9/src/solarwinds/solarwinds.cc
                       lib/rsxs-0.9/src/solarwinds/wind.cc)

set(PLASMA_SOURCES lib/rsxs-0.9/src/color.cc
                   lib/rsxs-0.9/src/common.cc
                   lib/rsxs-0.9/src/plasma/plasma.cc)

set(EUPHORIA_SOURCES lib/rsxs-0.9/src/color.cc
                     lib/rsxs-0.9/src/common.cc
                     lib/rsxs-0.9/src/euphoria/euphoria.cc
                     lib/rsxs-0.9/src/euphoria/wisp.cc)

set(DEPLIBS ${OPENGL_LIBRARIES}
            ${CMAKE_BINARY_DIR}/rsxs-prefix/src/rsxs-build/lib/libmisc.a)

build_addon(screensaver.rsxs.solarwinds SOLARWINDS DEPLIBS)
set_target_properties(screensaver.rsxs.solarwinds PROPERTIES
                      COMPILE_FLAGS -DPKGDATADIR="\\"/usr/share/xbmc/addons/screensaver.rsxs.solarwinds\\"")
add_dependencies(screensaver.rsxs.solarwinds rsxs)

build_addon(screensaver.rsxs.plasma PLASMA DEPLIBS)
set_target_properties(screensaver.rsxs.plasma PROPERTIES
                      COMPILE_FLAGS -DPKGDATADIR="\\"/usr/share/xbmc/addons/screensaver.rsxs.plasma\\"")
add_dependencies(screensaver.rsxs.plasma rsxs)

build_addon(screensaver.rsxs.euphoria EUPHORIA DEPLIBS)
set_target_properties(screensaver.rsxs.euphoria PROPERTIES
                      COMPILE_FLAGS -DPKGDATADIR="\\"/usr/share/xbmc/addons/screensaver.rsxs.euphoria\\"")
add_dependencies(screensaver.rsxs.euphoria rsxs)

include(CPack)
