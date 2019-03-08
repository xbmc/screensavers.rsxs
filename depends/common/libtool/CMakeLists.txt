cmake_minimum_required(VERSION 3.3)

project(libtool)

include(ExternalProject)

externalproject_add(libtool
  SOURCE_DIR ${CMAKE_SOURCE_DIR}
#  UPDATE_COMMAND autoreconf -vif
  CONFIGURE_COMMAND CC=${CMAKE_C_COMPILER} AR=${CMAKE_AR} ${CMAKE_SOURCE_DIR}/configure --prefix=${OUTPUT_DIR} --disable-shared --with-pic
  INSTALL_COMMAND ""
  BUILD_IN_SOURCE 1)

install(CODE "execute_process(COMMAND make install WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})")
