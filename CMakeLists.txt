cmake_minimum_required(VERSION 3.19)
project(Hbeat)




set(Hbeat_SOURCES
      Hbeat.cpp)
find_package(LibObs REQUIRED)
find_package(LSL 1.13.0 REQUIRED
      HINTS ${LSL_INSTALL_ROOT}
      "${CMAKE_CURRENT_LIST_DIR}/../../LSL/liblsl/build/"
      "${CMAKE_CURRENT_LIST_DIR}/../../LSL/liblsl/build/install"
      PATH_SUFFIXES share/LSL
			)
find_package(Threads)

add_library(Hbeat MODULE
      ${Hbeat_SOURCES})
target_link_libraries(Hbeat
      libobs
			LSL::lsl
      ${LIBOBS_LIBRARIES}
			${lsl-obs-plugin_PLATFORM_DEPS}
			)
