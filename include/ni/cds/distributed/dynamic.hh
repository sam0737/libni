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
#include <mutex>

#include <ni/cache_locality.hh>
#include <ni/random.hh>
#include <ni/sync/spinlock.hh>
#include <ni/tagged_ptr.hh>

namespace ni
{

/// \brief Locally linearizable dynamic distributed container
///
/// **Reference**
///
/// * Local linearizability for Concurrent -Type Data Structures.
///   A. Haas, T.A. Henzinger, A. Holzer, C.M. Kirsch, M. Lippautz, H. Payer,
///   A. Sezgin, A. Sokolova, and H. Veith. CoRR, abs/1502.07118, 2015.
///   http://arxiv.org/abs/1502.07118
///
/// \param T type of the backend
template <typename T>
class LLDynamicDistributed
{
private:
  using Backend = T;

  class NI_CACHELINE_ALIGNED Node : TaggedPtr<Backend>
  {
  public:
    Node();
    ~Node();

    Backend* backend() noexcept;
    // Indicates whether the backend is currently bound to a thread.
    bool alive() noexcept;
    void turn_off() noexcept;
  };

public:
  using Element = typename Backend::Element;

  class BackendPtr
  {
  public:
    BackendPtr() noexcept;

    Node* get() noexcept;
    Node* operator->() noexcept;
    void operator=(Node* ptr) noexcept;
    operator bool() const noexcept;

  private:
    Node* m_ptr;
  };

  explicit LLDynamicDistributed(size_t segment_capacity);
  ~LLDynamicDistributed();

  template <typename U>
  bool put(BackendPtr& local_backend, U&& element);

  bool get(BackendPtr& local_backend, Element* element);

  /// \brief Every thread that used this structure has to call this before exit.
  void deregister_thread(BackendPtr& local_backend);

private:
  Node** m_segment;
  size_t m_segment_capacity;
  size_t m_segment_length;
  size_t m_version;
  SpinLock m_lock;

  void remove_backend(size_t index);
};

template <typename T>
LLDynamicDistributed<T>::Node::Node()
  : TaggedPtr<Backend>(new Backend(), 1)
{
}

template <typename T>
LLDynamicDistributed<T>::Node::~Node()
{
  delete backend();
}

template <typename T>
typename LLDynamicDistributed<T>::Backend*
LLDynamicDistributed<T>::Node::backend() noexcept
{
  return this->value();
}

template <typename T>
bool LLDynamicDistributed<T>::Node::alive() noexcept
{
  return this->tag() == 1;
}

template <typename T>
void LLDynamicDistributed<T>::Node::turn_off() noexcept
{
  this->clear_tag();
}

template <typename T>
LLDynamicDistributed<T>::BackendPtr::BackendPtr() noexcept : m_ptr()
{
}

template <typename T>
typename LLDynamicDistributed<T>::Node*
LLDynamicDistributed<T>::BackendPtr::get() noexcept
{
  return m_ptr;
}

template <typename T>
typename LLDynamicDistributed<T>::Node* LLDynamicDistributed<T>::BackendPtr::
operator->() noexcept
{
  return get();
}

template <typename T>
void LLDynamicDistributed<T>::BackendPtr::operator=(Node* ptr) noexcept
{
  m_ptr = ptr;
}
template <typename T>
LLDynamicDistributed<T>::BackendPtr::operator bool() const noexcept
{
  return m_ptr != nullptr;
}

template <typename T>
LLDynamicDistributed<T>::LLDynamicDistributed(size_t segment_capacity)
  : m_segment()
  , m_segment_capacity(segment_capacity)
  , m_segment_length()
  , m_version()
  , m_lock()
{
  int rc = posix_memalign(reinterpret_cast<void**>(&m_segment), 64,
                          sizeof(Node*) * segment_capacity);
  if (rc)
    throw std::system_error(rc, std::system_category(), __func__);

  new (m_segment) Node* [segment_capacity]
  {
  };
}

template <typename T>
LLDynamicDistributed<T>::~LLDynamicDistributed()
{
  while (m_segment_length > 0)
  {
    --m_segment_length;
    Node* node = m_segment[m_segment_length];
    assert(node);
    delete node;
  }
  delete m_segment;
}

template <typename T>
template <typename U>
bool LLDynamicDistributed<T>::put(BackendPtr& local_backend, U&& element)
{
  if (!local_backend)
  {
    std::lock_guard<SpinLock> lock(m_lock);
    if (m_segment_length >= m_segment_capacity)
    {
      for (size_t i = 0; i < m_segment_length; ++i)
      {
        if (m_segment[i]->alive() == 0 && m_segment[i]->backend()->empty())
          remove_backend(i);
      }
    }
    if (m_segment_length >= m_segment_capacity)
      return false;

    local_backend = m_segment[m_segment_length++] = new Node();
    ++m_version;
  }
  return local_backend->backend()->put(std::forward<U>(element));
}

template <typename T>
bool LLDynamicDistributed<T>::get(BackendPtr& local_backend, Element* element)
{
  if (local_backend && local_backend->backend()->get(element))
    return true;

  pcg32 rng;
  std::uniform_int_distribution<uint64_t>
    uniform_dist(0, std::numeric_limits<uint64_t>::max());

  size_t version;

RETRY:
  while (m_segment_length)
  {
    size_t start = uniform_dist(rng) % m_segment_length;
    version = m_version;

    typename Backend::State tails_states[m_segment_length];

    for (size_t i = 0; i < m_segment_length; ++i)
    {
      size_t index = (start + i) % m_segment_length;
      Node* node = m_segment[index];
      Backend* backend = node->backend();
      if (!backend)
        continue;
      if (backend->pop(element, &tails_states[i]))
        return true;
      if (!node->alive())
      {
        remove_backend(index);
        goto RETRY;
      }
    }

    if (m_version != version)
      continue;

    for (size_t i = 0; i < m_segment_length; ++i)
    {
      size_t index = (start + i) % m_segment_length;
      Backend* backend = m_segment[index]->backend();
      if (!backend)
        continue;
      if (backend->tail_state() != tails_states[i])
        goto RETRY;
    }

    break;
  }

  return false;
}

template <typename T>
void LLDynamicDistributed<T>::deregister_thread(BackendPtr& local_backend)
{
  if (!local_backend)
    return;

  Node* node = local_backend.get();
  local_backend = nullptr;
  node->turn_off();
  if (!node->backend()->empty())
    return;

  for (size_t i = 0; i < m_segment_length; ++i)
  {
    if (m_segment[i] == node)
    {
      std::lock_guard<SpinLock> lock(m_lock);
      remove_backend(i);
      break;
    }
  }
}

template <typename T>
void LLDynamicDistributed<T>::remove_backend(size_t index)
{
  Node* node = m_segment[index];
  if (!node || node->alive() || !node->backend()->empty())
    return;

  delete node;
  --m_segment_length;
  m_segment[index] = m_segment[m_segment_length];
  m_segment[m_segment_length] = nullptr;
  ++m_version;
}

} // namespace ni
