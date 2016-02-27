// Copyright (C) 2016 Zhe Wang <0x1998@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
#include <ni/exception.hh>

#include <cxxabi.h>

namespace ni
{

namespace details
{

thread_local bool GET_EH_GLOBALS_CALLED = false;

// Declaration:
//
// struct __cxa_eh_globals {
//  __cxa_exception* caughtExceptions;
//  unsigned int uncaughtExceptions;
//};

} // namespace details

unsigned int uncaught_exceptions()
{
  using namespace __cxxabiv1;
  __cxa_eh_globals* globals;
  if (details::GET_EH_GLOBALS_CALLED)
  {
    globals = __cxa_get_globals_fast();
  }
  else
  {
    globals = __cxa_get_globals();
    details::GET_EH_GLOBALS_CALLED = true;
  }
  return *reinterpret_cast<unsigned int*>(
    reinterpret_cast<__cxa_exception**>(globals) + 1);
}

} // namespace ni

