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
#include <utility>

#include <ni/cache_locality.hh>
#include <ni/tagged_ptr.hh>

namespace ni
{
/// \brief MPMC Michael-Scott lock-free queue
///
/// **Note**
/// This queue does not scale well when contention increases. If strict FIFO
/// semantic is not required, it can be used as a backend (partial queue) of a
/// distributed queue which can provide much better scalability.
///
/// **Reference**
///
/// * M. M. Michael and M. L. Scott. Simple, Fast, and Practical Non-blocking
/// and Blocking Concurrent Queue Algorithms. PODC '96.
///
/// \param T type of the elements
template <typename T>
class MSQueue
{
public:
  using Element = T;

  class Node
  {
  public:
    using Ptr = TaggedPtr<Node>;
    using AtomicPtr = AtomicTaggedPtr<Ptr>;

    T value;
    AtomicPtr next;

    explicit Node(const T& item);
    explicit Node(T&& item);
  };

  enum PopResult
  {
    /// The pop operation succeeded
    Success,
    /// The queue is empty
    EmptyQueue,
    /// The provided head tag is invalid
    Failure
  };

  MSQueue();
  MSQueue(const MSQueue&) = delete;
  MSQueue& operator==(const MSQueue&) = delete;
  ~MSQueue();

  /// \return true if the queue is empty
  bool empty() const noexcept;

  /// \brief Push new element into the queue
  ///
  /// \param element Element to push
  template <typename U>
  void push(U&& element);

  /// \brief Try to push a new element into the queue
  ///
  /// \param element Element to push
  ///
  /// \return true on success
  template <typename U>
  bool try_push(U&& element, uint16_t old_tail_tag);

  /// \brief Pop an element from the queue
  ///
  /// \param [out] element Location to store the popped element (if the queue
  ///                      is not empty)
  /// \param [out] state Location to store the state of the queue if `state` is
  ///                    not null.
  ///
  /// \return false if the queue is empty
  bool pop(T* element, uint16_t* state = nullptr);

  /// \brief Try to pop an element from the queue
  ///
  /// \param [out] state Location to store the state of the queue
  ///
  /// \return See `PopResult`
  PopResult try_pop(T* element, uint16_t old_head_tag, uint16_t* state);

private:
  using NodePtr = typename Node::Ptr;
  using AtomicNodePtr = typename Node::AtomicPtr;

  NI_CACHELINE_ALIGNED AtomicNodePtr m_head;
  NI_CACHELINE_ALIGNED AtomicNodePtr m_tail;

  NI_PADDING_AFTER(sizeof(m_tail));
};

template <typename T>
MSQueue<T>::Node::Node(const T& item)
  : value(item)
  , next()
{
}

template <typename T>
MSQueue<T>::Node::Node(T&& item)
  : value(std::move(item))
  , next()
{
}

template <typename T>
MSQueue<T>::MSQueue()
{
  Node* node = new Node(T());
  m_head.store(NodePtr(node), std::memory_order_relaxed);
  m_tail.store(NodePtr(node), std::memory_order_relaxed);
}

template <typename T>
MSQueue<T>::~MSQueue()
{
  Node* head = m_head.load(std::memory_order_relaxed).value();
  Node* tail = m_tail.load(std::memory_order_relaxed).value();
  while (head != tail)
  {
    Node* tmp = head;
    head = tmp->next.load(std::memory_order_relaxed).value();
    delete tmp;
  }
  delete tail;
}

template <typename T>
bool MSQueue<T>::empty() const noexcept
{
  NodePtr old_head;
  NodePtr old_tail;
  NodePtr next;
  while (true)
  {
    old_head = m_head.load(std::memory_order_relaxed);
    old_tail = m_tail.load(std::memory_order_relaxed);
    next = old_head.value()->next.load(std::memory_order_relaxed);
    if (m_head.load(std::memory_order_relaxed) == old_head)
      return old_head.value() == old_tail.value() && next.value() == nullptr;
  }
}

template <typename T>
template <typename U>
void MSQueue<T>::push(U&& element)
{
  Node* node = new Node(std::forward<U>(element));
  NodePtr old_tail = m_tail.load(std::memory_order_relaxed);
  NodePtr next;

  while (true)
  {
    next = old_tail.value()->next.load(std::memory_order_relaxed);

    if (m_tail.load(std::memory_order_relaxed) != old_tail)
    {
      old_tail = m_tail.load(std::memory_order_relaxed);
      continue;
    }

    if (next.value() == nullptr)
    {
      if (old_tail.value()->next.compare_exchange_weak(
            next, NodePtr(node, next.tag() + 1), std::memory_order_release))
      {
        m_tail.compare_exchange_weak(old_tail,
                                     NodePtr(node, old_tail.tag() + 1),
                                     std::memory_order_release);
        return;
      }
      old_tail = m_tail.load(std::memory_order_relaxed);
    }
    else
    {
      m_tail.compare_exchange_weak(old_tail,
                                   NodePtr(next.value(), old_tail.tag() + 1),
                                   std::memory_order_release);
    }
  }
}

template <typename T>
template <typename U>
bool MSQueue<T>::try_push(U&& element, uint16_t old_tail_tag)
{
  NodePtr old_tail = m_tail.load(std::memory_order_relaxed);
  if (old_tail.tag() == old_tail_tag)
  {
    NodePtr next = old_tail.value()->next.load(std::memory_order_relaxed);
    if (next.value() == nullptr)
    {
      Node* node = new Node(std::forward<U>(element));
      if (old_tail.value()->next.compare_exchange_strong(
            next, NodePtr(node, next.tag() + 1), std::memory_order_release))
      {
        m_tail.compare_exchange_strong(old_tail,
                                       NodePtr(node, old_tail.tag() + 1),
                                       std::memory_order_release);
        return true;
      }
      delete node;
    }
    else
    {
      m_tail.compare_exchange_strong(old_tail,
                                     NodePtr(next.value(), old_tail.tag() + 1),
                                     std::memory_order_release);
    }
  }
  return false;
}

template <typename T>
bool MSQueue<T>::pop(T* element, uint16_t* state)
{
  NodePtr old_head;
  NodePtr old_tail;
  NodePtr next;

  while (true)
  {
    old_head = m_head.load(std::memory_order_relaxed);
    old_tail = m_tail.load(std::memory_order_relaxed);
    next = old_head.value()->next.load(std::memory_order_relaxed);

    if (m_head.load(std::memory_order_relaxed) != old_head)
      continue;

    if (old_head.value() == old_tail.value())
    {
      if (next.value() == nullptr)
      {
        if (state != nullptr)
          *state = old_tail.tag();
        return false;
      }
      m_tail.compare_exchange_weak(old_tail,
                                   NodePtr(next.value(), old_tail.tag() + 1),
                                   std::memory_order_release);
    }
    else
    {
      *element = next.value()->value;
      if (m_head.compare_exchange_weak(old_head, NodePtr(next.value(),
                                                         old_head.tag() + 1),
                                       std::memory_order_release))
      {
        if (state != nullptr)
          *state = old_head.tag();
        delete old_head.value();
        return true;
      }
    }
  }
}

template <typename T>
typename MSQueue<T>::PopResult MSQueue<T>::try_pop(T* element,
                                                   uint16_t old_head_tag,
                                                   uint16_t* state)
{
  NodePtr old_head = m_head.load(std::memory_order_relaxed);
  NodePtr old_tail = m_tail.load(std::memory_order_relaxed);
  NodePtr next = old_head.value()->next.load(std::memory_order_relaxed);

  if (old_head.tag() == old_head_tag)
  {
    if (old_head.value() == old_tail.value())
    {
      if (next.value() == nullptr)
      {
        *state = old_tail.tag();
        return PopResult::EmptyQueue;
      }
      m_tail.compare_exchange_strong(old_tail,
                                     NodePtr(next.value(), old_tail.tag() + 1),
                                     std::memory_order_release);
    }
    else
    {
      *element = next.value()->value;
      if (m_head.compare_exchange_strong(old_head, NodePtr(next.value(),
                                                           old_head.tag() + 1),
                                         std::memory_order_release))
      {
        delete old_head.value();
        return PopResult::Success;
      }
    }
  }
  return PopResult::Failure;
}

} // namespace ni
