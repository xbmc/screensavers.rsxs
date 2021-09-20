cmake_minimum_required(VERSION 3.5)

project(kodiOpenGL)

set(CMAKE_POSITION_INDEPENDENT_CODE 1)

set(SOURCES Texture.cpp
            ErrorCheck.cpp)

set(HEADERS Texture.h
            ErrorCheck.h)

add_library(kodiOpenGL STATIC ${SOURCES} ${HEADERS})
