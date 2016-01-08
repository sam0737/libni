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
#include <ni/logging/logger.hh>

#include <ni/logging/log_message.hh>
#include <ni/logging/message_bus.hh>
#include <ni/logging/sink.hh>

namespace ni
{
namespace logging
{

Logger::Logger(LogSeverity level, OverflowStrategy overflow_strategy)
  : m_level(level)
  , m_output()
  , m_overflow_strategy(overflow_strategy)
  , m_sinks()
{
}

Logger::~Logger()
{
  flush();
}

void
Logger::connect(MessageBus& msg_bus)
{
  m_output = &msg_bus;
}

void
Logger::add_sink(std::unique_ptr<Sink>&& sink)
{
  m_sinks.emplace_back(std::move(sink));
}

void
Logger::log(std::unique_ptr<LogMessage>&& message) noexcept
{
  thread_local static MessageBus::Channel* channel =
      m_output->get_input_channel();
  std::unique_ptr<LogMessage> msg = std::move(message);
  assert(msg);

  while (true)
  {
    if (channel->push(msg.get()))
    {
      msg.release();
      m_output->notify();
      return;
    }

    if (m_overflow_strategy == OverflowStrategy::DropMessage)
      return;
  }
}

void
Logger::save(LogMessage* message)
{
  assert(message);
  for (auto& sink: m_sinks)
  {
    assert(sink.get());
    sink->write(message);
  }
}


void
Logger::flush()
{
  for (auto& sink: m_sinks)
  {
    sink->flush();
  }
}

} // namespace logging
} // namespace ni
