#include <thread>

#include <ni/logging/capture.hh>
#include <ni/logging/log_service.hh>
#include <ni/logging/logger.hh>
#include <ni/logging/logging.hh>
#include <ni/logging/sink.hh>
#include <catch.hpp>

using namespace ni;

TEST_CASE("Logging")
{
  using namespace ni::logging;
  LogService service(/*queue_size=*/ 16);
  {
    std::unique_ptr<FileSink> sink(new FileSink(LogSeverity::Debug));
    sink->open("ni-logger.log");
    std::unique_ptr<Logger> logger(new Logger(LogSeverity::Debug,
                                              OverflowStrategy::Retry));
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
    threads.emplace_back([logger] {
      for (size_t i = 0; i < 100; ++i)
        LOG_INFO(logger) << "a message: " << i;
    });
  }
  for (auto& th: threads)
    th.join();
  service.stop();
}
