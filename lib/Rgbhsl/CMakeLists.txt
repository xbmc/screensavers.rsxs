cmake_minimum_required(VERSION 3.0)

project(Rgbhsl)

set(CMAKE_POSITION_INDEPENDENT_CODE 1)

set(SOURCES Rgbhsl.cpp)

set(HEADERS Rgbhsl.h)

add_library(Rgbhsl STATIC ${SOURCES} ${HEADERS})
target_include_directories(Rgbhsl PUBLIC ${CMAKE_CURRENT_LIST_DIR}/..)
