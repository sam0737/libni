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
#include <x86intrin.h>
#include <atomic>
#include <type_traits>

namespace ni
{

// This has to be POD
struct SpinLock
{
  bool locked;

  bool try_lock() noexcept;
  void lock() noexcept;
  void unlock() noexcept;

private:
  std::atomic<bool>* self() noexcept;
};

inline bool SpinLock::try_lock() noexcept
{
  bool tmp = false;
  return self()->compare_exchange_strong(tmp, true, std::memory_order_acquire,
                                         std::memory_order_relaxed);
}

inline void SpinLock::lock() noexcept
{
  std::atomic<bool>* flag = self();
  while (true)
  {
    while (flag->load(std::memory_order_relaxed))
      __pause();

    bool tmp = false;
    if (flag->compare_exchange_weak(tmp, true, std::memory_order_acquire,
                                    std::memory_order_relaxed))
      return;
  }
}

inline void SpinLock::unlock() noexcept
{
  self()->store(false, std::memory_order_release);
}

inline std::atomic<bool>* SpinLock::self() noexcept
{
  return reinterpret_cast<std::atomic<bool>*>(&locked);
}

static_assert(std::is_pod<SpinLock>::value, "SpinLock should be a POD type");

} // namespace ni
