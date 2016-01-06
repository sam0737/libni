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
