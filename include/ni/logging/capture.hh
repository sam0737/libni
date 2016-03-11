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
#include <ni/logging/log_message.hh>
#include <ni/logging/logger.hh>
#include <ni/logging/logging.hh>

namespace ni
{
namespace logging
{

#define FOR_ALL_INPUT_OVERLOAD_TYPES(X)                                        \
  X(int)                                                                       \
  X(unsigned)                                                                  \
  X(long)                                                                      \
  X(unsigned long)                                                             \
  X(long long)                                                                 \
  X(unsigned long long)                                                        \
  X(double)                                                                    \
  X(long double)                                                               \
  X(char)                                                                      \
  X(ni::fmt::StringRef)

class Capture
{
public:
  Capture(Logger* logger, LogSeverity severity, const char* file, int line);
  ~Capture();

  template <typename... Args>
  void log(ni::fmt::CStringRef fmt, Args&&... args);

#define DECLARE_INPUT_OP_OVERLOAD(TYPE) Capture& operator<<(TYPE value);
  FOR_ALL_INPUT_OVERLOAD_TYPES(DECLARE_INPUT_OP_OVERLOAD)
#undef DECLARE_INPUT_OP_OVERLOAD

  template <typename T, typename Spec, typename FillChar>
  Capture& operator<<(ni::fmt::IntFormatSpec<T, Spec, FillChar> spec);

  template <typename StrChar>
  Capture& operator<<(const ni::fmt::StrFormatSpec<StrChar>& spec);

  template <typename T>
  Capture& operator<<(T&& value);

private:
  Logger* m_logger;
  bool m_enabled;
  std::unique_ptr<LogMessage> m_message;
};

inline Capture::Capture(Logger* logger, LogSeverity severity, const char* file,
                        int line)
  : m_logger(logger)
{
  m_enabled = logger && severity >= logger->level();
  if (!m_enabled)
    return;

  m_message.reset(new LogMessage(logger, severity));

  struct timespec now_ts;
  // use CLOCK_REALTIME if needed
  if (clock_gettime(CLOCK_REALTIME_COARSE, &now_ts))
    NI_FATAL(NI_PERROR, "clock_gettime");

  struct tm now_tm;
  localtime_r(&now_ts.tv_sec, &now_tm);

  m_message->writer << 1900 + now_tm.tm_year << '-'
                    << ni::fmt::pad(now_tm.tm_mon + 1, 2, '0') << '-'
                    << ni::fmt::pad(now_tm.tm_mday, 2, '0') << ' '
                    << ni::fmt::pad(now_tm.tm_hour, 2, '0') << ':'
                    << ni::fmt::pad(now_tm.tm_min, 2, '0') << ':'
                    << ni::fmt::pad(now_tm.tm_sec, 2, '0') << '.'
                    << ni::fmt::pad(now_ts.tv_nsec / 1'000'000, 3, '0') << ' '
                    << LOG_SEVERITY_SHORT_NAMES[static_cast<int>(severity)]
                    << " [" << file << ':' << line << "] ";
}

inline Capture::~Capture()
{
  if (m_enabled)
  {
    m_message->writer << '\n';
    m_logger->log(std::move(m_message));
  }
}

template <typename... Args>
inline void Capture::log(ni::fmt::CStringRef fmt, Args&&... args)
{
  if (m_enabled)
    m_message->writer.write(fmt, std::forward<Args>(args)...);
}

#define DEFINE_INPUT_OP_OVERLOAD(TYPE)                                         \
                                                                               \
  inline Capture& Capture::operator<<(TYPE value)                              \
  {                                                                            \
    if (m_enabled)                                                             \
      m_message->writer << value;                                              \
    return *this;                                                              \
  }

FOR_ALL_INPUT_OVERLOAD_TYPES(DEFINE_INPUT_OP_OVERLOAD)

#undef DEFINE_INPUT_OP_OVERLOAD

#undef FOR_ALL_INPUT_OVERLOAD_TYPES

template <typename T, typename Spec, typename FillChar>
inline Capture& Capture::operator<<(
  ni::fmt::IntFormatSpec<T, Spec, FillChar> spec)
{
  if (m_enabled)
    m_message->writer << spec;
  return *this;
}

template <typename StrChar>
inline Capture& Capture::operator<<(const ni::fmt::StrFormatSpec<StrChar>& spec)
{
  if (m_enabled)
    m_message->writer << spec;
  return *this;
}

template <typename T>
inline Capture& Capture::operator<<(T&& value)
{
  if (m_enabled)
    m_message->writer.write("{}", std::forward<T>(value));
  return *this;
}

} // namespace logging
} // namespace ni
