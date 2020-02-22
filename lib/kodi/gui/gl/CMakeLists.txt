cmake_minimum_required(VERSION 3.5)

project(kodiOpenGL)

set(CMAKE_POSITION_INDEPENDENT_CODE 1)

set(SOURCES Texture.cpp
            ErrorCheck.cpp)

set(HEADERS Texture.h
            ErrorCheck.h)

add_library(kodiOpenGL STATIC ${SOURCES} ${HEADERS})
if(NOT GLM_FOUND)
  add_dependencies(kodiOpenGL glm)
endif()
if(NOT GLI_FOUND)
  add_dependencies(kodiOpenGL gli)
endif()
target_include_directories(kodiOpenGL PUBLIC ${GLI_INCLUDE_DIR} ${GLM_INCLUDE_DIR})
