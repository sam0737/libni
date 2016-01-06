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
#include <unordered_map>

#include <ni/logging/log_worker.hh>
#include <ni/logging/message_bus.hh>
#include <ni/string_view.hh>

namespace ni
{
namespace logging
{
class Logger;

// `LogService` owns and manages loggers and the background worker thread.
//
// Decouple the implementation from the singleton interface, allowing it to be
// used in locator pattern.
class LogService
{
public:
  LogService(size_t queue_size);
  ~LogService();
  bool add_logger(string_view name, std::unique_ptr<Logger>&& logger);
  Logger* get(const std::string& name);
  void start(pthread_attr_t* attrs = nullptr);
  void stop();

private:
  static constexpr size_t MAX_SPINS = 2048;

  MessageBus m_message_bus;
  std::unordered_map<std::string, std::unique_ptr<Logger>> m_loggers;
  LogWorker m_worker;
};

// Raised when `<Logger>* <LogService::get(string_view name)>` is called with an
// unknown logger name
class LoggerNotFound: public std::logic_error
{
public:
  explicit LoggerNotFound(string_view logger);
};

} // namespace logging
} // namespace ni
