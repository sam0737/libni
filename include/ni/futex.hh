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
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <linux/futex.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <atomic>

namespace ni
{

struct Futex : public std::atomic<int32_t>
{
  explicit Futex(int32_t value = 0) noexcept;

  /// \brief Puts the thread to sleep if this->load() == expected.
  /// \return Returns true if this->load() != expected or when it has consumed
  ///         a wake() event, false for any other return (signal, or spurious
  ///         wakeup).
  bool wait(int32_t expected, int wait_mask = -1) noexcept;

  /// \brief Wakens up to count waiters where (wait_mask & wake_mask) != 0.
  /// \return Returns the number of awoken threads.
  int wake(int count = INT_MAX, int wake_mask = -1) noexcept;
};

inline
Futex::Futex(int32_t value) noexcept
  : std::atomic<int32_t>(value)
{
}

inline bool
Futex::wait(int32_t expected, int wait_mask) noexcept
{
  int rv = syscall(SYS_futex, this,           // addr1
                   FUTEX_WAIT_BITSET_PRIVATE, // op
                   expected,                  // val
                   nullptr,                   // timeout
                   nullptr,                   // addr2
                   wait_mask);                // val3
  return (rv == 0 || errno == EWOULDBLOCK);
}

inline int
Futex::wake(int count, int wake_mask) noexcept
{
  assert(count > 0);
  int rv = syscall(SYS_futex, this,           // addr1
                   FUTEX_WAKE_BITSET_PRIVATE, // op
                   count,                     // val
                   nullptr,                   // timeout
                   nullptr,                   // addr2
                   wake_mask);                // val3
  return rv;
}

} // namespace ni
