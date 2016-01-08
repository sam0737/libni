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

file(GLOB_RECURSE test_headers *.hh)
set(CATCH_DIR ${deps}/catch/single_include)

add_library(test_driver OBJECT
  ${test_headers} # for QtCreator
  test.cc
)

target_include_directories(test_driver PUBLIC ${CATCH_DIR})

target_compile_options(test_driver PUBLIC
  -g
  -Wall
  -Wextra
  -Wno-unused-parameter
  -Wno-unused-variable
  -Wfatal-errors
)

function(add_tests)
  foreach(name ${ARGV})
    set(target ${name}_test)
    add_executable(${target}
      ${target}.cc
      $<TARGET_OBJECTS:test_driver>
    )
    target_include_directories(${target} PUBLIC ${CATCH_DIR})
    target_link_libraries(${target}
      ni
    )
    add_test(${name} ${target})
  endforeach(name)
endfunction(add_tests)

add_subdirectory(cds)
add_subdirectory(hash)

add_tests(
  logging
  tagged_ptr
)
