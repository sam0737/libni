#include <ni/logging/message_bus.hh>

namespace ni
{
namespace logging
{

MessageBus::MessageBus(size_t queue_size)
  : m_queue_size(queue_size)
  , m_channels()
  , m_pending_channels(4)
  , m_futex()
{
}

MessageBus::Channel*
MessageBus::get_input_channel()
{
  ChannelPtr channel(new Channel(m_queue_size));
  while (!m_pending_channels.push(channel.get()))
    ;
  return channel.release();
}

void
MessageBus::collect_pending_channels()
{
  ChannelPtr channel;
  while (true)
  {
    channel.reset(m_pending_channels.pop());
    if (!channel)
      break;
    m_channels.emplace_back(std::move(channel));
  }
}

void
MessageBus::notify() noexcept
{
  if (!m_futex.load(std::memory_order_acquire) &&
      !m_futex.exchange(1, std::memory_order_acq_rel))
    m_futex.wake(1);
}

void
MessageBus::wait() noexcept
{
  m_futex.store(0, std::memory_order_release);
  int rv;
  do rv = m_futex.wait(0);
  while (rv != 0 && errno == EINTR);
}

} // namespace logging
} // namespace ni
