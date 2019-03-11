cmake_minimum_required(VERSION 3.0)

project(Implicit)

set(CMAKE_POSITION_INDEPENDENT_CODE 1)

set(SOURCES impCapsule.cpp
            impCubeTables.cpp
            impCubeVolume.cpp
            impEllipsoid.cpp
            impHexahedron.cpp
            impKnot.cpp
            impRoundedHexahedron.cpp
            impShape.cpp
            impSphere.cpp
            impSurface.cpp
            impTorus.cpp)

set(HEADERS impCapsule.h
            impCrawlPoint.h
            impCubeTables.h
            impCubeVolume.h
            impEllipsoid.h
            impHexahedron.h
            impKnot.h
            impRoundedHexahedron.h
            impShape.h
            impSphere.h
            impSurface.h
            impTorus.h)

add_library(Implicit STATIC ${SOURCES} ${HEADERS})
target_include_directories(Implicit PUBLIC ${CMAKE_CURRENT_LIST_DIR}/..)
