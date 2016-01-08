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
#include <ni/logging/log_service.hh>

#include <ni/format.hh>
#include <ni/logging/logger.hh>

namespace ni
{
namespace logging
{

LogService::LogService(size_t queue_size)
  : m_message_bus(queue_size)
  , m_loggers()
  , m_worker(m_message_bus)
{
}

LogService::~LogService()
{
  stop();
}

bool
LogService::add_logger(string_view name, std::unique_ptr<Logger>&& logger)
{
  Logger* tmp = logger.get();
  auto result = m_loggers.emplace(std::make_pair(name.to_string(),
                                                 std::move(logger)));
  if (result.second)
    tmp->connect(m_message_bus);
  return result.second;
}

Logger*
LogService::get(const std::string& name)
{
  auto it = m_loggers.find(name);
  if (it == m_loggers.end())
    throw LoggerNotFound(name);
  return it->second.get();
}

void
LogService::start(pthread_attr_t* attrs)
{
  m_worker.start(attrs);
}

void
LogService::stop()
{
  m_worker.stop();
}

LoggerNotFound::LoggerNotFound(string_view logger)
  : std::logic_error(ni::fmt::format("Logger \"{}\" not found", logger))
{
}

} // namespace logging
} // namespace ni
