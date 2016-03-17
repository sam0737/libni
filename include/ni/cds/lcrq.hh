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
// LIABILITY, WHETHER IN AN ACTION OF CONTRACTORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
#pragma once
#include <algorithm>
#include <atomic>
#include <memory>
#include <utility>

#include <ni/cache_locality.hh>
#include <ni/tagged_ptr.hh>

namespace ni
{
/// \brief LCRQ - Linked Concurrent Ring Queue
///
/// **Note**
/// This queue does not scale beyond about 20 threads.
///
/// **Reference**
///
/// * Fast Concurrent Queues for x86 Processors.  Adam Morrison and Yehuda
///   Afek.  PPoPP 2013.  http://mcg.cs.tau.ac.il/projects/lcrq/
///
/// \param T type of the elements
template <uint64_t EMPTY = 0, size_t RING_SIZE_POWER = 10>
class LCRQ
{
public:
  LCRQ();
  LCRQ(const LCRQ&) = delete;
  LCRQ& operator==(const LCRQ&) = delete;
  ~LCRQ();

  /// \brief Push new element into the queue
  ///
  /// \param element uint64_t to push
  bool push(uint64_t element);

  /// \brief Pop an element from the queue
  ///
  /// \param [out] element Location to store the popped element (if the queue
  ///                      is not empty)
  ///
  /// \return false if the queue is empty
  bool pop(uint64_t* element);

private:
  using uint128_t = unsigned __int128;

  static constexpr size_t RING_SIZE = 1ULL << RING_SIZE_POWER;
  static constexpr size_t TAG_MASK = 1ULL << 63;
  static constexpr size_t INDEX_MASK = ~TAG_MASK;

  static_assert(sizeof(uint64_t) <= sizeof(uint64_t),
                "T can not fit into 8 bytes");
  static_assert(RING_SIZE_POWER > 0, "RING_SIZE_POWER can not be 0");
  static_assert(
    RING_SIZE_POWER <= 63,
    "Index type I is not large enough to hold RING_SIZE_POWER bits");

  struct alignas(uint128_t) RingNode
  {
    // Tag bit denotes whether dequeuing from the node is unsafe.
    uint64_t index;
    uint64_t value;
  };

  class NI_CACHELINE_ALIGNED RingQueue
  {
  public:
    NI_CACHELINE_ALIGNED std::atomic<uint64_t> head;
    // Tag bit denotes whether the queue is closed.
    NI_CACHELINE_ALIGNED std::atomic<uint64_t> tail;
    NI_CACHELINE_ALIGNED std::atomic<RingQueue*> next;

    RingQueue() noexcept;
    std::atomic<RingNode>& operator[](size_t index) noexcept;
    void fix_state() noexcept;

  private:
    struct NI_CACHELINE_ALIGNED Cell
    {
      std::atomic<RingNode> node;
    };

    NI_CACHELINE_ALIGNED std::array<Cell, RING_SIZE> nodes;
  };

  NI_CACHELINE_ALIGNED std::atomic<RingQueue*> m_head;
  NI_CACHELINE_ALIGNED std::atomic<RingQueue*> m_tail;
  thread_local static std::unique_ptr<RingQueue> m_new_queue;

  static uint64_t get_index(uint64_t index);
  static uint64_t get_tag(uint64_t index);
  static uint64_t set_tag(uint64_t index, uint64_t tag = TAG_MASK);
  static bool test_and_set_tag(std::atomic<uint64_t>& target);
};

#define LCRQ_TPL template <uint64_t EMPTY, size_t RING_SIZE_POWER>
#define LCRQ_CLS LCRQ<EMPTY, RING_SIZE_POWER>

LCRQ_TPL thread_local std::unique_ptr<typename LCRQ_CLS::RingQueue>
  LCRQ_CLS::m_new_queue;

LCRQ_TPL LCRQ_CLS::LCRQ()
  : m_head(new RingQueue())
  , m_tail(m_head.load(std::memory_order_relaxed))
{
}

LCRQ_TPL LCRQ_CLS::~LCRQ()
{
  RingQueue* head = m_head.load(std::memory_order_relaxed);
  while (head)
  {
    RingQueue* tmp = head;
    head = head->next.load(std::memory_order_relaxed);
    delete tmp;
  }
}

LCRQ_TPL bool LCRQ_CLS::push(uint64_t element)
{
  int attempts_to_close = 0;

  while (true)
  {
    RingQueue* queue = m_tail.load(std::memory_order_acq_rel);

    RingQueue* next = queue->next.load(std::memory_order_relaxed);
    if (NI_UNLIKELY(next))
    {
      m_tail.compare_exchange_strong(queue, next, std::memory_order_release,
                                     std::memory_order_relaxed);
      continue;
    }

    uint64_t tail = queue->tail.fetch_add(1, std::memory_order_acq_rel);

    // The queue is closed.
    if (get_tag(tail))
    {
    ADD_QUEUE:
      if (!m_new_queue)
        m_new_queue.reset(new RingQueue());

      m_new_queue->tail.store(1, std::memory_order_relaxed);
      (*m_new_queue)[0].store(RingNode{0, element}, std::memory_order_relaxed);

      RingQueue* expected = nullptr;
      if (queue->next.compare_exchange_weak(expected, m_new_queue.get(),
                                            std::memory_order_release,
                                            std::memory_order_relaxed))
      {
        m_tail.compare_exchange_strong(queue, m_new_queue.get(),
                                       std::memory_order_release,
                                       std::memory_order_relaxed);
        m_new_queue.release();
        return true;
      }

      continue;
    }

    std::atomic<RingNode>& node = (*queue)[tail];

    RingNode n = node.load(std::memory_order_relaxed);

    if (NI_LIKELY(n.value == EMPTY) && NI_LIKELY(get_index(n.index) <= tail) &&
        (NI_LIKELY(!get_tag(n.index)) ||
         queue->head.load(std::memory_order_acquire) < tail))
    {
      RingNode expected{n.index, EMPTY};
      if (node.compare_exchange_strong(expected, RingNode{tail, element},
                                       std::memory_order_release,
                                       std::memory_order_relaxed))
        return true;
    }

    uint64_t head = queue->head.load(std::memory_order_relaxed);
    if (NI_UNLIKELY((tail - head) > RING_SIZE))
    {
      ++tail;
      if (++attempts_to_close < 10
            ? queue->tail.compare_exchange_strong(tail, tail | TAG_MASK,
                                                  std::memory_order_release,
                                                  std::memory_order_relaxed)
            : test_and_set_tag(queue->tail))
        goto ADD_QUEUE;
    }
  }
}

LCRQ_TPL bool LCRQ_CLS::pop(uint64_t* element)
{
  while (true)
  {
    RingQueue* queue = m_head.load(std::memory_order_consume);

    uint64_t head = queue->head.fetch_add(1, std::memory_order_acq_rel);
    std::atomic<RingNode>& node = (*queue)[head];

    uint64_t tail = 0;
    int retry = 0;

    while (true)
    {
      RingNode expected = node.load(std::memory_order_relaxed);
      uint64_t index = get_index(expected.index);
      uint64_t unsafe = get_tag(expected.index);

      if (NI_UNLIKELY(index > head))
        break;

      if (NI_LIKELY(expected.value != EMPTY))
      {
        if (NI_LIKELY(index == head))
        {
          // Transition to empty
          if (node.compare_exchange_weak(
                expected, RingNode{set_tag(head + RING_SIZE, unsafe), EMPTY},
                std::memory_order_release, std::memory_order_relaxed))
          {
            *element = expected.value;
            return true;
          }
        }
        else
        {
          if (node.compare_exchange_weak(expected, RingNode{set_tag(index),
                                                            expected.value},
                                         std::memory_order_release,
                                         std::memory_order_relaxed))
            break;
        }
      }
      else
      {
        if ((retry & ((1ULL << 10) - 1)) == 0)
          tail = queue->tail.load(std::memory_order_relaxed);

        RingNode desired{set_tag(head + RING_SIZE, unsafe), EMPTY};
        if (NI_UNLIKELY(unsafe))
        {
          if (node.compare_exchange_weak(expected, desired,
                                         std::memory_order_release,
                                         std::memory_order_relaxed))
            break;
        }
        else if (tail < head + 1 || retry > 200000 || get_tag(tail))
        {
          if (node.compare_exchange_weak(expected, desired,
                                         std::memory_order_release,
                                         std::memory_order_relaxed))
          {
            if (retry > 200000 && tail > RING_SIZE)
              test_and_set_tag(queue->tail);
            break;
          }
        }
        else
        {
          ++retry;
        }
      }
    }

    if (get_index(queue->tail.load(std::memory_order_relaxed)) > head + 1)
      continue;

    queue->fix_state();

    RingQueue* next = queue->next.load(std::memory_order_relaxed);
    // Queue is empty
    if (!next)
      return false;

    if (get_index(queue->tail.load(std::memory_order_relaxed)) > head + 1)
      continue;

    if (m_head.compare_exchange_strong(queue, next, std::memory_order_release,
                                       std::memory_order_relaxed))
      delete queue;
  }
}

LCRQ_TPL LCRQ_CLS::RingQueue::RingQueue() noexcept : head(0),
                                                     tail(0),
                                                     next(nullptr)
{
  for (size_t i = 0; i < RING_SIZE; ++i)
    std::atomic_init(&nodes[i].node, RingNode{i, EMPTY});
}

LCRQ_TPL std::atomic<typename LCRQ_CLS::RingNode>& LCRQ_CLS::RingQueue::
operator[](size_t index) noexcept
{
  return nodes[index & (RING_SIZE - 1)].node;
}

LCRQ_TPL void LCRQ_CLS::RingQueue::fix_state() noexcept
{
  while (true)
  {
    uint64_t t = tail.load(std::memory_order_relaxed);
    uint64_t h = head.load(std::memory_order_relaxed);

    if (NI_UNLIKELY(tail.load(std::memory_order_relaxed) != t))
      continue;

    if (h <= t ||
        tail.compare_exchange_weak(t, h, std::memory_order_release,
                                   std::memory_order_relaxed))
      return;
  }
}

LCRQ_TPL uint64_t LCRQ_CLS::get_index(uint64_t index)
{
  return index & INDEX_MASK;
}

LCRQ_TPL uint64_t LCRQ_CLS::get_tag(uint64_t index)
{
  return index & TAG_MASK;
}

LCRQ_TPL uint64_t LCRQ_CLS::set_tag(uint64_t index, uint64_t tag)
{
  return index | tag;
}

LCRQ_TPL bool LCRQ_CLS::test_and_set_tag(std::atomic<uint64_t>& target)
{
  char result;
  asm volatile("lock btsq $63, %0;"
               "setnc %1"
               : "+m"(*reinterpret_cast<uint64_t*>(&target)), "=a"(result)
               :
               : "cc");
  return result;
}

#undef LCRQ_TPL
#undef LCRQ_CLS

} // namespace ni
