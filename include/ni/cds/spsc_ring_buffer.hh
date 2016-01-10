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
#include <atomic>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <system_error>
#include <type_traits>

#include <ni/cache_locality.hh>
#include <ni/mpl/unit.hh>

namespace ni
{
namespace details
{
template <typename T, T Empty, typename Fill, typename Enabled = void>
struct SPSCRingBufferFiller
{
  static constexpr Unit value = Unit();
  using type = Unit;
};

template <typename T, T Empty>
struct SPSCRingBufferFiller<T, Empty, T, T>
{
  static constexpr int value = reinterpret_cast<int>(Empty);
  using type = int;
};

template <typename T, T Empty, typename Fill>
struct SPSCRingBufferFiller<
  T, Empty, Fill, typename std::enable_if<
                    std::is_integral<typename Fill::value_type>::value>::type>
{
  static constexpr int value = static_cast<int>(Fill::value);
  using type = int;
};

} // namespace details

/// \brief Wait-free single producer single consumer ring buffer
///
/// **Reference**
///
/// * Massimo Torquati, "Single-Producer/Single-Consumer Queue on Shared Cache
/// Multi-Core Systems", TR-10-20, Computer Science Department, University of
/// Pisa Italy,2010
/// ( http://compass2.di.unipi.it/TR/Files/TR-10-20.pdf.gz )
///
/// * M. Aldinucci, M. Danelutto, P. Kilpatrick, M. Meneghin, and M. Torquati,
/// "An Efficient Unbounded Lock-Free Queue for Multi-core Systems," in Proc. of
/// 18th Intl. Euro-Par 2012 Parallel Processing, Rhodes Island, Greece, 2012,
/// pp. 662-673. doi:10.1007/978-3-642-32820-6_65
///
/// \param T type of the elements
/// \param Empty `empty` or `null` value of type T (defaults to `T()`)
/// \param Fill
///
template <typename T, T Empty = T(), typename Fill = T>
class SPSCRingBuffer
{
public:
  /// \brief Create a new ring buffer
  /// \param size the maximum size of the ring buffer, must be power of two (
  ///        and greater than 1 )
  explicit SPSCRingBuffer(size_t size);
  SPSCRingBuffer(const SPSCRingBuffer&) = delete;
  SPSCRingBuffer& operator==(const SPSCRingBuffer&) = delete;
  ~SPSCRingBuffer();

  /// \return size of the whole ring buffer
  size_t size() const;

  /// \return number of elements currently inside the ring buffer
  size_t len() const;

  /// \return true if the buffer is empty
  bool empty() const;

  /// \return true if there are empty slots
  bool available() const;

  /// \brief Push new element into the ring buffer
  /// \param element element to push
  /// \return whether the element was pushed into the ring buffer
  bool push(const T& element);

  /// \brief Pop an element from the ring buffer
  /// \return nullptr the ring buffer is empty
  T pop();

  /// \return the next element to be popped without removing it
  T top() const;

  /// \brief Clear the ring buffer
  void reset();

private:
  NI_CACHELINE_ALIGNED std::atomic<size_t> m_write_index;
  NI_CACHELINE_ALIGNED std::atomic<size_t> m_read_index;
  const size_t m_size;
  const size_t m_mask;
  std::atomic<T>* m_buf;

  // Fill out the cache line to prevent false sharing with other allocations
  NI_PADDING_AFTER(sizeof(m_size) + sizeof(m_mask) + sizeof(m_buf));

  using Filler = details::SPSCRingBufferFiller<T, Empty, Fill>;
  void clear(Unit, bool initialized = true);
  void clear(int value, bool initialized = true);
};

template <typename T, T Empty, typename Fill>
SPSCRingBuffer<T, Empty, Fill>::SPSCRingBuffer(size_t size)
  : m_write_index()
  , m_read_index()
  , m_size(size)
  , m_mask(size - 1)
  , m_buf()
{
  assert((size > 1) && (size & (size - 1)) == 0 &&
         "size must be a power of two");

  int rc = posix_memalign(reinterpret_cast<void**>(&m_buf),
                          NI_CACHELINE_SIZE<size_t>, sizeof(T) * size);
  if (rc)
    throw std::system_error(rc, std::system_category(), __func__);

  clear(Filler::value, false);
}

template <typename T, T Empty, typename Fill>
SPSCRingBuffer<T, Empty, Fill>::~SPSCRingBuffer()
{
  delete[] m_buf;
}

template <typename T, T Empty, typename Fill>
size_t SPSCRingBuffer<T, Empty, Fill>::size() const
{
  return m_size;
}

template <typename T, T Empty, typename Fill>
size_t SPSCRingBuffer<T, Empty, Fill>::len() const
{
  size_t w = m_write_index;
  size_t r = m_read_index;
  if (w > r)
    return w - r;
  if (w < r)
    return m_size - r + w;
  if (w == r && m_buf[w].load(std::memory_order_relaxed) == Empty)
    return 0;
  return m_size;
}

template <typename T, T Empty, typename Fill>
bool SPSCRingBuffer<T, Empty, Fill>::empty() const
{
  return m_buf[m_read_index].load(std::memory_order_acquire) == Empty;
}

template <typename T, T Empty, typename Fill>
bool SPSCRingBuffer<T, Empty, Fill>::available() const
{
  return m_buf[m_write_index].load(std::memory_order_acquire) == Empty;
}

template <typename T, T Empty, typename Fill>
bool SPSCRingBuffer<T, Empty, Fill>::push(const T& element)
{
  if (!available())
    return false;

  m_buf[m_write_index].store(element, std::memory_order_release);
  m_write_index = (m_write_index + 1) & m_mask;
  return true;
}

template <typename T, T Empty, typename Fill>
T SPSCRingBuffer<T, Empty, Fill>::pop()
{
  T element = m_buf[m_read_index].load(std::memory_order_acquire);
  if (element == Empty)
    return Empty;

  m_buf[m_read_index].store(Empty, std::memory_order_release);
  m_read_index = (m_read_index + 1) & m_mask;
  return element;
}

template <typename T, T Empty, typename Fill>
T SPSCRingBuffer<T, Empty, Fill>::top() const
{
  return m_buf[m_read_index].load(std::memory_order_acquire);
}

template <typename T, T Empty, typename Fill>
void SPSCRingBuffer<T, Empty, Fill>::reset()
{
  m_read_index = m_write_index = 0;
  clear(Filler::value);
}

template <typename T, T Empty, typename Fill>
void SPSCRingBuffer<T, Empty, Fill>::clear(Unit, bool initialized)
{
  if (!initialized)
    new (m_buf) std::atomic<T>[m_size]
    {
    };
  for (size_t i = 0; i < m_size; ++i)
    m_buf[i].store(Empty, std::memory_order_release);
}

template <typename T, T Empty, typename Fill>
void SPSCRingBuffer<T, Empty, Fill>::clear(int value, bool)
{
  memset(m_buf, value, sizeof(T) * m_size);
}

/// \brief Convenient template alias for pointers
template <typename T>
using SPSCPtrRingBuffer =
  SPSCRingBuffer<T*, nullptr, std::integral_constant<int, 0>>;

} // namespace ni
