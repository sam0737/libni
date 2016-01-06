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

# set C++ std version
add_compile_options(-std=gnu++1z)

ExternalProject_Add(cppformat
  GIT_REPOSITORY    https://github.com/cppformat/cppformat.git
  GIT_TAG           804ad8f4df7e01259374a1d2f458775f1b2d1164
  CMAKE_ARGS        -DFMT_TEST=OFF -DFMT_DOC=OFF -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
  BUILD_BYPRODUCTS  ${CMAKE_BINARY_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}cppformat${CMAKE_STATIC_LIBRARY_SUFFIX}
)
ExternalProject_Get_Property(cppformat BUILD_BYPRODUCTS)
add_library(cppformat::static STATIC IMPORTED)
set_target_properties(cppformat::static PROPERTIES IMPORTED_LOCATION ${BUILD_BYPRODUCTS})
add_dependencies(cppformat::static cppformat)

set(BACKWARD_TESTS FALSE)
set(STACK_WALKING_UNWIND TRUE)
set(STACK_DETAILS_DW TRUE)
add_subdirectory(${deps}/backward)
macro(add_backward target)
  target_include_directories(${target} PRIVATE ${deps}/backward)
  set_property(TARGET ${target} APPEND PROPERTY LINK_LIBRARIES "dw")
endmacro()

# config.hh
if (NOT DEFINED CACHELINE_SIZE)
  find_program(getconf getconf)
  mark_as_advanced(getconf)

  if(getconf)
    execute_process(
      COMMAND ${getconf} LEVEL1_DCACHE_LINESIZE
      OUTPUT_VARIABLE L1_DCACHE_LINESIZE
    )
  endif()

  if(${L1_DCACHE_LINESIZE} AND NOT ${L1_DCACHE_LINESIZE} STREQUAL "undefined\n")
    string(STRIP ${L1_DCACHE_LINESIZE} CACHELINE_SIZE)
  else()
    set(CACHELINE_SIZE 64)
  endif()
endif()

if (NOT DEFINED LOG_FILE_PATH_IDX)
  set(LOG_FILE_PATH_IDX 0)
endif()

configure_file(
  ${CMAKE_SOURCE_DIR}/include/ni/config.hh.in
  ${CMAKE_BINARY_DIR}/include/ni/config.hh
)

# for QtCreator
file(GLOB_RECURSE headers
  ${CMAKE_SOURCE_DIR}/include/*.hh
  ${CMAKE_BINARY_DIR}/include/*.hh
  ${deps}/*.hh
)

# Include/link directories
include_directories(
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_BINARY_DIR}/include
)
link_directories(${CMAKE_BINARY_DIR}/lib)

# Source directories
add_subdirectory(src)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()

# Installation directories
install(DIRECTORY   ${CMAKE_BINARY_DIR}/bin
                    ${CMAKE_BINARY_DIR}/lib
                    ${CMAKE_BINARY_DIR}/man
                    ${CMAKE_BINARY_DIR}/share
        DESTINATION ${CMAKE_INSTALL_PREFIX}
        USE_SOURCE_PERMISSIONS)

# docs
