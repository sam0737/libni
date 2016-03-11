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

# targets
file(GLOB_RECURSE src_headers *.hh)

add_library(ni STATIC
  # for QtCreator
  ${headers}
  ${src_headers}
  ${BACKWARD_ENABLE}
  exception.cc
  hash/jump_consistent_hash.cc
  logging/common.cc
  logging/logger.cc
  logging/logging.cc
  logging/log_service.cc
  logging/log_worker.cc
  logging/message_bus.cc
  logging/sink.cc
)

add_backward(ni)

target_include_directories(ni PUBLIC
  ${CMAKE_SOURCE_DIR}/include/
)

target_compile_options(ni PUBLIC
  -g
  -Wall
  -Wextra
  -Wno-unused-parameter
  -Wno-unused-variable
  -Wfatal-errors
)

target_link_libraries(ni PUBLIC
  cppformat::static
  pthread
)

cotire(ni)

install(TARGETS ni ARCHIVE DESTINATION lib)
