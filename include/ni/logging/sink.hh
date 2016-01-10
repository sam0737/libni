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
#include <stdio.h>

#include <ni/logging/common.hh>
#include <ni/logging/log_message.hh>
#include <ni/string_view.hh>

namespace ni
{
namespace logging
{

struct LogMessage;

class Sink
{
public:
  explicit Sink(LogSeverity level) noexcept;
  virtual ~Sink(){};
  virtual void write(LogMessage* msg) = 0;
  virtual void flush() = 0;

protected:
  LogSeverity m_level;

  bool should_accept(LogMessage* msg) const noexcept;
};

inline Sink::Sink(LogSeverity level) noexcept : m_level(level)
{
}

inline bool Sink::should_accept(LogMessage* msg) const noexcept
{
  assert(msg);
  return msg->severity >= m_level;
}

// A simple sink for files (which should not be accessed outside of this class)
class FileSink : public Sink
{
public:
  FileSink(LogSeverity level);
  ~FileSink() override;
  // Returns the fd on success so it can be preserved when daemonizing. Returns
  // -1 on error.
  int open(int fd);
  int open(string_view filename);
  // @inherit
  void write(LogMessage* msg) override;
  // @inherit
  void flush() override;

private:
  FILE* m_stream;
};

inline FileSink::FileSink(LogSeverity level)
  : Sink(level)
  , m_stream()
{
}

} // namespace logging
} // namespace ni
