project(visualizations.rsxs)

cmake_minimum_required(VERSION 2.6)

enable_language(CXX)

find_package(kodi REQUIRED)
find_package(OpenGL REQUIRED)

set(rsxs_dir lib/rsxs-1.0)

include_directories(${OpenGL_INCLUDE_DIR}
                    ${KODI_INCLUDE_DIR}
                    ${CMAKE_BINARY_DIR}/rsxs-prefix/src/rsxs-build
                    ${PROJECT_SOURCE_DIR}/${rsxs_dir}/src
                    ${PROJECT_SOURCE_DIR}/${rsxs_dir}/src/euphoria
                    ${PROJECT_SOURCE_DIR}/${rsxs_dir}/src/plasma
                    ${PROJECT_SOURCE_DIR}/${rsxs_dir}/src/solarwinds)
                        
include(ExternalProject)
set(update_command "")
if(BOOTSTRAP_IN_TREE OR NOT DEFINED BOOTSTRAP_IN_TREE)
  set(update_command UPDATE_COMMAND autoreconf -vif WORKING_DIRECTORY <SOURCE_DIR>)
endif()
externalproject_add(rsxs SOURCE_DIR ${PROJECT_SOURCE_DIR}/${rsxs_dir}
                    CONFIGURE_COMMAND ${PROJECT_SOURCE_DIR}/${rsxs_dir}/configure
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

set(SOLARWINDS_SOURCES ${rsxs_dir}/src/color.cc
                       ${rsxs_dir}/src/common.cc
                       ${rsxs_dir}/src/solarwinds/solarwinds.cc
                       ${rsxs_dir}/src/solarwinds/wind.cc)

set(PLASMA_SOURCES ${rsxs_dir}/src/color.cc
                   ${rsxs_dir}/src/common.cc
                   ${rsxs_dir}/src/plasma/plasma.cc)

set(EUPHORIA_SOURCES ${rsxs_dir}/src/color.cc
                     ${rsxs_dir}/src/common.cc
                     ${rsxs_dir}/src/euphoria/euphoria.cc
                     ${rsxs_dir}/src/euphoria/wisp.cc)

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
