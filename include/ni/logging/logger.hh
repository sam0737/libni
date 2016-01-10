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
#include <memory>
#include <vector>

#include <ni/logging/common.hh>

namespace ni
{
namespace logging
{
class Formatter;
class LogMessage;
class MessageBus;
class Sink;

//- Note:
// Pass sink parameters by rvalue references instead of by values. See
// http://goo.gl/HCu8Sf
class Logger
{
public:
  Logger(LogSeverity level, OverflowStrategy overflow_strategy);
  ~Logger();

  void connect(MessageBus& msg_bus);

  // Get logger level severity
  LogSeverity level() const noexcept;

  void add_sink(std::unique_ptr<Sink>&& sink);

  // Log the `message`. Should be called in destructor of `Capture`
  void log(std::unique_ptr<LogMessage>&& message) noexcept;
  // Save the `message` to all the sinks
  void save(LogMessage* message);
  // Flush all the sinks
  void flush();

private:
  LogSeverity m_level;
  MessageBus* m_output;
  OverflowStrategy m_overflow_strategy;
  std::vector<std::unique_ptr<Sink>> m_sinks;
};

inline LogSeverity Logger::level() const noexcept
{
  return m_level;
}

} // namespace logging
} // namespace ni
