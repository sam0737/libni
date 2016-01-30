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
#pragma once
#include <time.h>
#include <x86intrin.h>

namespace ni
{

class Pause
{
public:
  Pause(uint32_t max_spins) noexcept;

  void operator()() noexcept;
  void reset() noexcept;

private:
  uint32_t m_max_spins;
  uint32_t m_spins;
};

Pause::Pause(uint32_t max_spins) noexcept
  : m_max_spins(max_spins)
  , m_spins(0)
{
}

void Pause::operator()() noexcept
{
  if (m_spins < m_max_spins)
  {
    ++m_spins;
    __pause();
  }
  else
  {
    struct timespec ts = { 0, 500000 };
    nanosleep(&ts, nullptr);
  }
}

void Pause::reset() noexcept
{
  m_spins = 0;
}

} // namespace ni
