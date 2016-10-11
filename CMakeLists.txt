# Copyright (C) 2016 Zhe Wang <0x1998@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

# See `cmake --help-policy CMP0058` and
# https://cmake.org/gitweb?p=cmake.git;a=blob;f=Tests/CustomCommandByproducts/CMakeLists.txt;hb=HEAD
cmake_minimum_required(VERSION 3.3)

# Project definition
project(libni C CXX)

# Third party dependencies
set(deps ${PROJECT_SOURCE_DIR}/thirdparty)
set(deps_headers ${CMAKE_BINARY_DIR}/include/ni/thirdparty)
file(MAKE_DIRECTORY ${deps_headers})
file(COPY ${deps}/pcg/include/ DESTINATION ${deps_headers})

# CMake modules
include(CTest)
include(ExternalProject)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${deps}/cotire/CMake)
include(cotire)

# External dependencies
set_directory_properties(PROPERTIES EP_PREFIX ${CMAKE_BINARY_DIR})
file(MAKE_DIRECTORY
  ${CMAKE_BINARY_DIR}/lib/
)

# Set C++ std version
add_compile_options(-std=gnu++1z)

# Prepare thirdparty dependencies
ExternalProject_Add(cppformat
# GIT_REPOSITORY    https://github.com/cppformat/cppformat.git
# GIT_TAG           a99891e7a5f6c8a164d271b1aa0f09e7d13c7de4
  SOURCE_DIR        ${CMAKE_SOURCE_DIR}/thirdparty/cppformat
  CMAKE_ARGS        -DFMT_TEST=OFF -DFMT_DOC=OFF -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
  BUILD_BYPRODUCTS  ${CMAKE_BINARY_DIR}/lib/libcppformat.a
)
ExternalProject_Add_Step(cppformat bundle_headers
  DEPENDEES install
  COMMENT "copying include/cppformat to include/ni/thirdparty/"
  COMMAND rm -rf ${deps_headers}/cppformat
  COMMAND cp -a <INSTALL_DIR>/include/cppformat ${deps_headers}/
)
ExternalProject_Get_Property(cppformat BUILD_BYPRODUCTS)
set(cppformat_static_file ${BUILD_BYPRODUCTS})
add_library(cppformat_static STATIC IMPORTED)
set_target_properties(cppformat_static
  PROPERTIES IMPORTED_LOCATION ${cppformat_static_file})
add_dependencies(cppformat_static cppformat)

set(BACKWARD_TESTS FALSE)
set(STACK_WALKING_UNWIND TRUE)
set(STACK_DETAILS_DW TRUE)
add_subdirectory(${deps}/backward)
macro(add_backward target)
  target_include_directories(${target} PRIVATE ${deps}/backward)
  set_property(TARGET ${target} APPEND PROPERTY LINK_LIBRARIES "dw")
endmacro()

# config.hh
if (NOT DEFINED LOG_FILE_PATH_IDX)
  set(LOG_FILE_PATH_IDX 0)
endif()

configure_file(
  ${CMAKE_SOURCE_DIR}/include/ni/config.hh.in
  ${CMAKE_BINARY_DIR}/include/ni/config.hh
)

# for QtCreator
file(GLOB_RECURSE headers
  ${CMAKE_SOURCE_DIR}/include/*.h
  ${CMAKE_SOURCE_DIR}/include/*.hh
  ${CMAKE_BINARY_DIR}/include/*.h
  ${CMAKE_BINARY_DIR}/include/*.hh
  ${deps}/*.hh
  ${deps}/*.hpp
)

# Include/link directories
include_directories(
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_BINARY_DIR}/include
  ${deps}/pcg/include
)
link_directories(${CMAKE_BINARY_DIR}/lib)

# Source directories
add_subdirectory(src)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()

# Installation directories
install(DIRECTORY include/ni DESTINATION include)
install(DIRECTORY ${CMAKE_BINARY_DIR}/include/ni DESTINATION include)
install(FILES ${cppformat_static_file} DESTINATION lib)

# docs
