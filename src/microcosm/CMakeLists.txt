project(screensaver.rsxs.microcosm)

cmake_minimum_required(VERSION 3.0)

enable_language(CXX)

message(STATUS "--------------------------------------------------------------------------------")
message(STATUS "Create \"${CMAKE_PROJECT_NAME}\"")

unset(ADDON_DEPENDS)
unset(USED_SOURCES)
set_property(DIRECTORY PROPERTY COMPILE_DEFINITIONS ${BASE_DEFINITIONS})

set(MICROCOSM_SOURCES ${CMAKE_CURRENT_LIST_DIR}/main.cpp
                      ${CMAKE_CURRENT_LIST_DIR}/gizmo.cpp
                      ${CMAKE_CURRENT_LIST_DIR}/mirrorBox.cpp
                      ${CMAKE_CURRENT_LIST_DIR}/rsCamera.cpp
                      ${CMAKE_CURRENT_LIST_DIR}/texture1d.cpp)

set(MICROCOSM_HEADERS ${CMAKE_CURRENT_LIST_DIR}/main.h
                      ${CMAKE_CURRENT_LIST_DIR}/brain.h
                      ${CMAKE_CURRENT_LIST_DIR}/cube.h
                      ${CMAKE_CURRENT_LIST_DIR}/cubesAndCapsules.h
                      ${CMAKE_CURRENT_LIST_DIR}/cylinder.h
                      ${CMAKE_CURRENT_LIST_DIR}/flower.h
                      ${CMAKE_CURRENT_LIST_DIR}/gizmo.h
                      ${CMAKE_CURRENT_LIST_DIR}/grinder.h
                      ${CMAKE_CURRENT_LIST_DIR}/knotAndSpheres.h
                      ${CMAKE_CURRENT_LIST_DIR}/knotAndTorus.h
                      ${CMAKE_CURRENT_LIST_DIR}/kube2.h
                      ${CMAKE_CURRENT_LIST_DIR}/kube3.h
                      ${CMAKE_CURRENT_LIST_DIR}/kube4.h
                      ${CMAKE_CURRENT_LIST_DIR}/kube.h
                      ${CMAKE_CURRENT_LIST_DIR}/metaballs.h
                      ${CMAKE_CURRENT_LIST_DIR}/mirrorBox.h
                      ${CMAKE_CURRENT_LIST_DIR}/octahedron.h
                      ${CMAKE_CURRENT_LIST_DIR}/orbit.h
                      ${CMAKE_CURRENT_LIST_DIR}/ringOfTori.h
                      ${CMAKE_CURRENT_LIST_DIR}/rings.h
                      ${CMAKE_CURRENT_LIST_DIR}/rsCamera.h
                      ${CMAKE_CURRENT_LIST_DIR}/spheresAndCapsules.h
                      ${CMAKE_CURRENT_LIST_DIR}/stringOfEllipsoids.h
                      ${CMAKE_CURRENT_LIST_DIR}/tennis.h
                      ${CMAKE_CURRENT_LIST_DIR}/tetrahedron.h
                      ${CMAKE_CURRENT_LIST_DIR}/texture1d.h
                      ${CMAKE_CURRENT_LIST_DIR}/torusBox.h
                      ${CMAKE_CURRENT_LIST_DIR}/triangleOfSpheres.h
                      ${CMAKE_CURRENT_LIST_DIR}/ufo.h)

build_addon(screensaver.rsxs.microcosm MICROCOSM DEPLIBS)
