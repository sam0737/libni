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
#include <thread>

#include <catch.hpp>

#include <ni/logging/capture.hh>
#include <ni/logging/log_service.hh>
#include <ni/logging/logger.hh>
#include <ni/logging/logging.hh>
#include <ni/logging/sink.hh>

using namespace ni;
using namespace ni::logging;

TEST_CASE("Logging")
{
  LogService service(/*queue_size=*/16);
  {
    auto logger =
      std::make_unique<Logger>(LogSeverity::Debug, OverflowStrategy::Retry);
    auto sink = std::make_unique<FileSink>(LogSeverity::Debug);
    sink->open("/tmp/ni-logger.log");
    logger->add_sink(std::move(sink));
    service.add_logger("console", std::move(logger));
    service.start();
  }
  Logger* logger = service.get("console");
  std::string msg("logging starts");
  string_view sv(msg.data() + 3, 5);
  LOG_INFO(logger) << sv;

  std::vector<std::thread> threads;
  for (size_t i = 0; i < 3; ++i)
  {
    threads.emplace_back([logger]
                         {
                           for (size_t i = 0; i < 100; ++i)
                             LOG_INFO(logger) << "a message: " << i;
                         });
  }
  for (auto& th : threads)
    th.join();
  service.stop();
}
