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
#include <thread>

#include <catch.hpp>

#include <ni/cds/spsc_buffer_list.hh>

using namespace ni;

TEST_CASE("SPSCRingBuffer")
{
  SPSCRingBuffer<int, -1> ringbuf(8);
  REQUIRE(ringbuf.empty());
  REQUIRE(ringbuf.available());
  REQUIRE(ringbuf.len() == 0);

  std::thread t1([&]
                 {
                   for (int i = 0; i < 100; ++i)
                   {
                     while (!ringbuf.push(i))
                       pthread_yield();
                   }
                 });

  std::thread t2([&]
                 {
                   for (int i = 0; i < 100; ++i)
                   {
                     int val;
                     while ((val = ringbuf.pop()) == -1)
                       pthread_yield();
                     REQUIRE(val == i);
                   }
                 });

  t1.join();
  t2.join();
}
