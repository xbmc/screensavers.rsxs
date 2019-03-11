project(ImageMagick)

cmake_minimum_required(VERSION 3.3)

find_package(JPEG REQUIRED)
find_package(TIFF REQUIRED)
find_package(PNG REQUIRED)
find_package(LTDL)

include_directories(${LTDL_INCLUDE_DIRS} ${JPEG_INCLUDE_DIRS} ${PNG_INCLUDE_DIR} ${TIFF_INCLUDE_DIR})

message(STATUS "--------------------------_> ${CMAKE_MODULE_PATH}")
# Force on travis where only Ubuntu 16.04 Xenial is available the use of gcc
# on Image Magick, otherwise brings it configure errors!
if(DEFINED ENV{TRAVIS_COMPILER})
  if($ENV{TRAVIS_COMPILER} STREQUAL "clang")
    set(OVERRIDE_COMPILER CC=gcc CXX=g++)
  endif()
endif()

include(ExternalProject)
ExternalProject_Add(ImageMagick
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    UPDATE_COMMAND ""
                    CONFIGURE_COMMAND ${OVERRIDE_COMPILER} <SOURCE_DIR>/configure
                      --prefix=${OUTPUT_DIR}
                      --enable-static
                      --disable-shared
                      --disable-opencl
                      --with-pic
                      --without-x
                      --without-magick-plus-plus
                      --without-openexr
                      --without-openjp2
                      --without-pango
                      --without-fftw
                      --without-fontconfig
                      --without-freetype
                      --without-djvu
                      --without-lcms
                      --without-jbig
                      --without-lqr
                      --without-bzlib
                      --without-lzma
                      --without-webp
                      --without-xml
                      --without-gvc
                    INSTALL_COMMAND ""
                    BUILD_IN_SOURCE 1)

install(CODE "execute_process(COMMAND make install WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})")
