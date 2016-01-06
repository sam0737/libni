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
#include <ni/cds/spsc_linked_list.hh>

namespace ni
{

template <typename T, T Empty = T(), typename Fill = T>
class SPSCBufferList
{
  using Buffer = SPSCRingBuffer<T, Empty, Fill>;
public:
  SPSCBufferList(size_t buffer_size, size_t cache_size, bool fill_cache = false);
  ~SPSCBufferList();

  bool empty() const;
  void push(const T& element);
  T pop();

private:
  NI_CACHELINE_ALIGNED Buffer* m_buf_to_read;
  NI_CACHELINE_ALIGNED Buffer* m_buf_to_write;
  const size_t m_buf_size;
  SPSCLinkedList<Buffer*> m_in_use;
  SPSCRingBuffer<Buffer*> m_cache;

  Buffer* next_to_read();
  Buffer* next_to_write(const size_t buf_size);
  void release(Buffer* buf);
  void reset();
};

template <typename T, T Empty, typename Fill>
SPSCBufferList<T, Empty, Fill>::SPSCBufferList(size_t buffer_size,
                                               size_t cache_size,
                                               bool fill_cache)
  : m_buf_to_read(new Buffer(buffer_size))
  , m_buf_to_write(m_buf_to_read)
  , m_buf_size(buffer_size)
  , m_in_use(cache_size)
  , m_cache(cache_size)
{
  if (fill_cache)
  {
    assert(buffer_size > 0);
    for (size_t i = 0; i < cache_size; ++i)
      m_cache.push(new Buffer(buffer_size));
  }
}

template <typename T, T Empty, typename Fill>
SPSCBufferList<T, Empty, Fill>::~SPSCBufferList()
{
  if (m_buf_to_read)
    delete m_buf_to_read;

  Buffer* buf;
  while ((buf = m_in_use.pop()))
    delete buf;

  while ((buf = m_cache.pop()))
    delete buf;
}

template <typename T, T Empty, typename Fill>
bool
SPSCBufferList<T, Empty, Fill>::empty() const
{
  return m_buf_to_read->empty() && m_buf_to_read == m_buf_to_write;
}

template <typename T, T Empty, typename Fill>
void
SPSCBufferList<T, Empty, Fill>::push(const T& element)
{
  if (!m_buf_to_write->available())
    m_buf_to_write = next_to_write(m_buf_size);

  m_buf_to_write->push(element);
}

template <typename T, T Empty, typename Fill>
T
SPSCBufferList<T, Empty, Fill>::pop()
{
  if (m_buf_to_read->empty())
  {
    if (m_buf_to_read == m_buf_to_write)
      return Empty;

    if (m_buf_to_read->empty())
    {
      Buffer* next = next_to_read();
      if (next)
      {
        release(m_buf_to_read);
        m_buf_to_read = next;
      }
    }
  }
  return m_buf_to_read->pop();
}

template <typename T, T Empty, typename Fill>
typename SPSCBufferList<T, Empty, Fill>::Buffer*
SPSCBufferList<T, Empty, Fill>::next_to_read()
{
  return m_in_use.pop();
}

template <typename T, T Empty, typename Fill>
typename SPSCBufferList<T, Empty, Fill>::Buffer*
SPSCBufferList<T, Empty, Fill>::next_to_write(const size_t buf_size)
{
  Buffer* buf = m_cache.pop();
  if (!buf)
    buf = new Buffer(buf_size);

  m_in_use.push(buf);
  return buf;
}

template <typename T, T Empty, typename Fill>
void
SPSCBufferList<T, Empty, Fill>::release(Buffer* buf)
{
  buf->reset();
  if (!m_cache.push(buf))
    delete buf;
}

template <typename T, T Empty, typename Fill>
void
SPSCBufferList<T, Empty, Fill>::reset()
{
  Buffer* buf;
  while ((buf = m_in_use.pop()))
    release(buf);
}

} // namespace ni
