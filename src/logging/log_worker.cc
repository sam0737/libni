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
#include <ni/logging/log_worker.hh>

#include <x86intrin.h>

#include <ni/logging/log_message.hh>
#include <ni/logging/logger.hh>
#include <ni/logging/message_bus.hh>

namespace ni
{
namespace logging
{

LogWorker::LogWorker(MessageBus& message_bus)
  : m_input(message_bus)
  , m_thread()
  , m_stopping()
{
}

LogWorker::~LogWorker()
{
  stop();
}

void LogWorker::start(pthread_attr_t* attrs)
{
  assert(!m_thread);
  pthread_create(&m_thread, attrs, LogWorker::start_thread, this);
  pthread_setname_np(m_thread, "Logging");
}

void* LogWorker::start_thread(void* args)
{
  static_cast<LogWorker*>(args)->run();
  return nullptr;
}

void LogWorker::run()
{
  size_t spins = 0;
  std::vector<Logger*> loggers;
  std::unique_ptr<LogMessage> msg;
  auto flush_loggers = [&]
  {
    for (Logger* logger : loggers)
      logger->flush();
  };

  while (true)
  {
    bool has_messages = false;
    loggers.clear();

    for (MessageBus::ChannelPtr& channel : m_input.channels())
    {
      msg.reset(channel->pop());
      if (!msg)
        continue;

      loggers.emplace_back(msg->logger);
      msg->logger->save(msg.get());
      msg = nullptr;
      has_messages = true;
    }

    if (has_messages)
    {
      flush_loggers();
      spins = 0;
      continue;
    }

    if (m_stopping.load(std::memory_order_acquire))
    {
      flush_loggers();
      return;
    }

    if (!spins)
      flush_loggers();

    if (spins < MAX_SPINS)
    {
      __pause();
      ++spins;
    }
    else
    {
      m_input.wait();
      spins = 0;
    }
  }
}

void LogWorker::stop()
{
  if (!m_thread)
    return;
  m_stopping.store(true, std::memory_order_release);
  m_input.notify();
  pthread_join(m_thread, nullptr);
  m_thread = 0;
}

} // namespace logging
} // namespace ni
