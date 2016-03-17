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

#include <ni/cds/lcrq.hh>

using namespace ni;

TEST_CASE("LCRQ-FIFO")
{
  LCRQ<> queue;

  std::atomic<uint64_t> in(0);
  std::vector<std::thread> threads;

  for (int i = 0; i < 1; ++i)
  {
    threads.emplace_back(
      [&]
      {
        for (int i = 0; i < 333; ++i)
        {
          queue.push(in.fetch_add(1, std::memory_order_relaxed));
        }
      });
  }

  threads.emplace_back(
    [&]
    {
      uint64_t sum = 0;
      for (int i = 0; i < 999; ++i)
      {
        uint64_t value;
        while (!queue.pop(&value))
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        sum += value;
      }
      REQUIRE(sum == (0 + 998) * 999 / 2);
    });

  for (auto& t : threads)
    t.join();
}
