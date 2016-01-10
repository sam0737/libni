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
#include <ni/cds/spsc_ring_buffer.hh>

namespace ni
{

template <typename T, T Empty = T(), typename Fill = T>
class SPSCLinkedList
{
public:
  explicit SPSCLinkedList(size_t cache_size = NI_CACHELINE_SIZE<size_t>,
                          bool fill_cache = false);
  ~SPSCLinkedList();
  void push(const T& element);
  T pop();

private:
  struct Node
  {
    T data;
    std::atomic<Node*> next;

    Node();
    explicit Node(const T& data, Node* next = nullptr);
    explicit Node(T&& data, Node* next = nullptr);
  };

  NI_CACHELINE_ALIGNED Node* m_head;
  NI_CACHELINE_ALIGNED Node* m_tail;
  SPSCPtrRingBuffer<Node> m_cache;

  // Fill out the cache line to prevent false sharing with other allocations
  NI_PADDING_AFTER(sizeof(m_cache));
};

template <typename T, T Empty, typename Fill>
SPSCLinkedList<T, Empty, Fill>::Node::Node()
  : data()
  , next()
{
}

template <typename T, T Empty, typename Fill>
SPSCLinkedList<T, Empty, Fill>::Node::Node(const T& data, Node* next)
  : data(data)
  , next(next)
{
}

template <typename T, T Empty, typename Fill>
SPSCLinkedList<T, Empty, Fill>::Node::Node(T&& data, Node* next)
  : data(std::move(data))
  , next(next)
{
}

template <typename T, T Empty, typename Fill>
SPSCLinkedList<T, Empty, Fill>::SPSCLinkedList(size_t cache_size,
                                               bool fill_cache)
  : m_head()
  , m_tail()
  , m_cache(cache_size)
{
  Node* n = new Node();
  m_head = m_tail = n;

  if (fill_cache)
  {
    for (size_t i = 0; i < cache_size; ++i)
    {
      Node* n = new Node();
      m_cache.push(n);
    }
  }
}

template <typename T, T Empty, typename Fill>
SPSCLinkedList<T, Empty, Fill>::~SPSCLinkedList()
{
  Node* p;
  while ((p = m_cache.pop()))
    delete p;

  while (m_head != m_tail)
  {
    p = m_head;
    m_head = m_head->next;
    delete p;
  }

  if (m_head)
    delete m_head;
}

template <typename T, T Empty, typename Fill>
void SPSCLinkedList<T, Empty, Fill>::push(const T& element)
{
  Node* n = m_cache.pop();
  if (!n)
    n = new Node(element);

  m_tail->next.store(n, std::memory_order_release);
  m_tail = n;
}

template <typename T, T Empty, typename Fill>
T SPSCLinkedList<T, Empty, Fill>::pop()
{
  Node* next = m_head->next.load(std::memory_order_acquire);
  if (!next)
    return Empty;

  const T element = next->data;
  Node* n = m_head;
  m_head = next;
  if (!m_cache.push(n))
    delete n;

  return element;
}

} // namespace ni
