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
