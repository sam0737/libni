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
