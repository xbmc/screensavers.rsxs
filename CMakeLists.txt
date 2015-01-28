project(visualizations.rsxs)

cmake_minimum_required(VERSION 2.6)

enable_language(CXX)

find_package(kodi REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PNG REQUIRED)

set(rsxs_dir lib/rsxs-1.0)

include_directories(${OpenGL_INCLUDE_DIR}
                    ${PNG_INCLUDE_DIR}
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
add_options(ALL_LANGUAGES ALL_BUILDS -fPIC)

add_library(rsxs_common STATIC ${rsxs_dir}/src/color.cc
                               ${rsxs_dir}/src/common.cc
                               ${rsxs_dir}/src/implicit.cc
                               ${rsxs_dir}/src/pngimage.cc)
add_dependencies(rsxs_common rsxs)

set(SOLARWINDS_SOURCES ${rsxs_dir}/src/solarwinds/solarwinds.cc
                       ${rsxs_dir}/src/solarwinds/wind.cc)

set(PLASMA_SOURCES ${rsxs_dir}/src/plasma/plasma.cc
                   src/plasma.cpp)

set(EUPHORIA_SOURCES ${rsxs_dir}/src/euphoria/euphoria.cc
                     ${rsxs_dir}/src/euphoria/wisp.cc
                     src/euphoria.cpp)

set(DEPLIBS ${OPENGL_LIBRARIES}
            ${PNG_LIBRARIES}
            ${CMAKE_BINARY_DIR}/rsxs-prefix/src/rsxs-build/lib/libmisc.a
            rsxs_common)

foreach(addon solarwinds plasma euphoria)
  string(TOUPPER ${addon} ADDON)
  build_addon(screensaver.rsxs.${addon} ${ADDON} DEPLIBS)
  add_dependencies(screensaver.rsxs.${addon} rsxs_common)
endforeach()

include(CPack)
