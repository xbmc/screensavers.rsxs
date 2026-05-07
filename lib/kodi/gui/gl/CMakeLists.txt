cmake_minimum_required(VERSION 3.18...4.0)

project(kodiOpenGL)

set(CMAKE_POSITION_INDEPENDENT_CODE 1)

set(SOURCES Texture.cpp
            ErrorCheck.cpp)

set(HEADERS Texture.h
            ErrorCheck.h)

add_library(kodiOpenGL STATIC ${SOURCES} ${HEADERS})
