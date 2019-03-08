cmake_minimum_required(VERSION 3.0)

project(rsMath)

set(CMAKE_POSITION_INDEPENDENT_CODE 1)

set(SOURCES rsMatrix.cpp
            rsQuat.cpp
            rsVec4.cpp
            rsVec.cpp)

set(HEADERS rsMath.h
            rsMatrix.h
            rsQuat.h
            rsTrigonometry.h
            rsVec4.h
            rsVec.h)

add_library(rsMath STATIC ${SOURCES} ${HEADERS})
target_include_directories(rsMath PUBLIC ${CMAKE_CURRENT_LIST_DIR}/..)
