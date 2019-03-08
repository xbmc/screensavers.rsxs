project(ImageMagick)

cmake_minimum_required(VERSION 3.3)

find_package(JPEG REQUIRED)
find_package(TIFF REQUIRED)
find_package(PNG REQUIRED)
find_package(LTDL)

include_directories(${LTDL_INCLUDE_DIRS} ${JPEG_INCLUDE_DIRS} ${PNG_INCLUDE_DIR} ${TIFF_INCLUDE_DIR})

# Force on travis where only Ubuntu 16.04 Xenial is available the use of gcc
# on Image Magick, otherwise brings it configure errors!
if(DEFINED ENV{TRAVIS_COMPILER})
  if($ENV{TRAVIS_COMPILER} STREQUAL "clang")
    set(OVERRIDE_COMPILER CC=gcc CXX=g++)
  endif()
endif()

file(READ "${CMAKE_SOURCE_DIR}/depends/common/imagemagick/imagemagick.txt" url)
separate_arguments(url)
list(GET url 1 url)
file(READ "${CMAKE_SOURCE_DIR}/depends/common/imagemagick/imagemagick.sha256" sha256)
separate_arguments(sha256)
string(STRIP ${sha256} sha256)

include(ExternalProject)
ExternalProject_Add(ImageMagick
                    URL ${url}
                    URL_HASH SHA256=${sha256}
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

ExternalProject_Get_Property(ImageMagick source_dir)

set(ImageMagick_INCLUDE_DIRS ${source_dir} CACHE INTERNAL "")
set(ImageMagick_LIBRARIES ${source_dir}/magick/.libs/libMagickCore-6.Q16.a
                           ${source_dir}/wand/.libs/libMagickWand-6.Q16.a
                           CACHE INTERNAL "")
