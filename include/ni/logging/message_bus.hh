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
#include <memory>
#include <vector>

#include <ni/cds/spsc_ring_buffer.hh>
#include <ni/futex.hh>

namespace ni
{
namespace logging
{
class LogMessage;

class MessageBus
{
public:
  using Channel = SPSCPtrRingBuffer<LogMessage>;
  using ChannelPtr = std::unique_ptr<Channel>;
  using Channels = std::vector<ChannelPtr>;
  using PendingChannels = SPSCPtrRingBuffer<Channel>;

  explicit MessageBus(size_t queue_size);
  Channel* get_input_channel();
  Channels& channels();
  void notify() noexcept;
  void wait() noexcept;

private:
  size_t m_queue_size;
  Channels m_channels;
  PendingChannels m_pending_channels;
  Futex m_futex;

  void collect_pending_channels();
};

inline MessageBus::Channels& MessageBus::channels()
{
  collect_pending_channels();
  return m_channels;
}

} // namespace logging
} // namespace ni
