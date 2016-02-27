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
#include <exception>
#include <type_traits>
#include <utility>

#include <ni/exception.hh>
#include <ni/preprocessor.hh>

namespace ni
{

enum class ScopeExit
{
  All,
  NoException,
  UncaughtException,
};

template <typename Fn>
class ScopeGuard
{
public:
  ScopeGuard(ScopeExit exit_condition, const Fn& fn);
  ScopeGuard(ScopeExit exit_condition, Fn&& fn);
  ScopeGuard(ScopeGuard&& other);

  ScopeGuard(const ScopeGuard&) = delete;
  ScopeGuard& operator=(const ScopeGuard&) = delete;
  void* operator new(size_t) = delete;

  ~ScopeGuard();
  void deactivate() noexcept;

private:
  ScopeExit m_exit_condition;
  Fn m_fn;
  bool m_active;
  unsigned m_uncaught_exceptions;
};

template <typename Fn>
ScopeGuard<Fn>::ScopeGuard(ScopeExit exit_condition, const Fn& fn)
  : m_exit_condition(exit_condition)
  , m_fn(fn)
  , m_active(true)
  , m_uncaught_exceptions(
      exit_condition == ScopeExit::All ? 0 : uncaught_exceptions())
{
}

template <typename Fn>
ScopeGuard<Fn>::ScopeGuard(ScopeExit exit_condition, Fn&& fn)
  : m_exit_condition(exit_condition)
  , m_fn(std::move(fn))
  , m_active(true)
  , m_uncaught_exceptions(
      exit_condition == ScopeExit::All ? 0 : uncaught_exceptions())
{
}

template <typename Fn>
ScopeGuard<Fn>::ScopeGuard(ScopeGuard&& other)
  : m_exit_condition(other.m_exit_condition)
  , m_fn(std::move(other.m_fn))
  , m_active(other.m_active)
  , m_uncaught_exceptions(other.m_uncaught_exceptions)
{
  other.deactivate();
}

template <typename Fn>
ScopeGuard<Fn>::~ScopeGuard()
{
  if (m_active)
  {
    if (m_exit_condition != ScopeExit::All)
    {
      unsigned uncaught_exceptions_now = uncaught_exceptions();
      if (m_exit_condition == ScopeExit::NoException)
      {
        if (uncaught_exceptions_now > m_uncaught_exceptions)
          return;
      }
      else if (m_exit_condition == ScopeExit::UncaughtException)
      {
        if (uncaught_exceptions_now <= m_uncaught_exceptions)
          return;
      }
    }
    m_fn();
  }
}

template <typename Fn>
void ScopeGuard<Fn>::deactivate() noexcept
{
  m_active = false;
}

template <typename Fn>
ScopeGuard<typename std::decay<Fn>::type> make_scope_guard(
  ScopeExit exit_condition, Fn&& fn)
{
  return ScopeGuard<typename std::decay<Fn>::type>(exit_condition,
                                                   std::forward<Fn>(fn));
}

template <typename Fn>
ScopeGuard<typename std::decay<Fn>::type> operator<<(ScopeExit exit_condition,
                                                     Fn&& fn)
{
  return ScopeGuard<typename std::decay<Fn>::type>(exit_condition,
                                                   std::forward<Fn>(fn));
}

#define NI_DEFER_ON(exit_condition)                                            \
  auto NI_NEW_VAR(ni_scope_guard_) = exit_condition <<

#define NI_DEFER NI_DEFER_ON(ni::ScopeExit::All)

} // namespace ni
