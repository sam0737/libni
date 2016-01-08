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
#include <algorithm>
#include <atomic>
#include <cassert>
#include <climits>

namespace ni
{

/// \brief 64-bit tagged pointer for storing a 16-bit tag and a 48-bit value.
template <typename T>
class TaggedPtr
{
public:
  using RawType = uint64_t;
  using Tag = uint16_t;
  using Value = T*;
  RawType raw_value;

  TaggedPtr() noexcept;
  explicit TaggedPtr(RawType raw) noexcept;
  explicit TaggedPtr(Value value) noexcept;
  TaggedPtr(Value value, Tag tag) noexcept;

  Value value() const noexcept;
  Tag tag() const noexcept;

  bool operator==(TaggedPtr other) const noexcept;
  bool operator!=(TaggedPtr other) const noexcept;

private:
  static constexpr size_t VALUE_BITS = 48;
  static constexpr size_t VALUE_MASK = (1ULL << VALUE_BITS) - 1;
};

template <typename T>
TaggedPtr<T>::TaggedPtr() noexcept
  : raw_value()
{
}

template <typename T>
TaggedPtr<T>::TaggedPtr(RawType raw) noexcept
  : raw_value(raw)
{
}

template <typename T>
TaggedPtr<T>::TaggedPtr(Value value) noexcept
  : raw_value(reinterpret_cast<uint64_t>(value))
{
  assert(this->value() == value);
}

template <typename T>
TaggedPtr<T>::TaggedPtr(Value value, Tag tag) noexcept
  : raw_value(*reinterpret_cast<RawType*>(&value) |
              (static_cast<RawType>(tag) << VALUE_BITS))
{
  assert(this->value() == value && this->tag() == tag);
}

template <typename T>
typename TaggedPtr<T>::Value
TaggedPtr<T>::value() const noexcept
{
  uint64_t tmp = raw_value & VALUE_MASK;
  return *reinterpret_cast<Value*>(&tmp);
}

template <typename T>
typename TaggedPtr<T>::Tag
TaggedPtr<T>::tag() const noexcept
{
  return static_cast<Tag>(raw_value >> VALUE_BITS);
}

template <typename T>
bool
TaggedPtr<T>::operator==(TaggedPtr other) const noexcept
{
  return raw_value == other.raw_value;
}

template <typename T>
bool
TaggedPtr<T>::operator!=(TaggedPtr other) const noexcept
{
  return !(*this == other);
}


/// \brief 64-bit tagged pointer with two tags. Size of the first tag is 16-bit.
///        Size of the second tag depends on the alignment of the value it
///        pointed to which defaults to the alignment of T.
template <typename T, typename T2 = uint8_t, size_t alignment = alignof(T)>
class DoubleTagPtr
{
public:
  using RawType = uint64_t;
  using Value = T*;
  using Tag1 = uint16_t;
  using Tag2 = T2;
  RawType raw_value;

  DoubleTagPtr() noexcept;
  explicit DoubleTagPtr(RawType raw) noexcept;
  explicit DoubleTagPtr(Value value) noexcept;
  DoubleTagPtr(Value value, Tag1 tag1, Tag2 tag2) noexcept;

  Value value() const noexcept;
  Tag1 tag1() const noexcept;
  Tag2 tag2() const noexcept;

  bool operator==(DoubleTagPtr other) const noexcept;
  bool operator!=(DoubleTagPtr other) const noexcept;

private:
  static constexpr size_t VALUE_BITS = 48;
  static constexpr size_t TAG2_MAX_BITS = __builtin_ctz(alignment);
  static constexpr size_t TAG2_BITS = std::min(VALUE_BITS, TAG2_MAX_BITS);
  static_assert(std::is_integral<Tag2>::value &&
                sizeof(Tag2) * CHAR_BIT >= TAG2_BITS,
                "T2 must be integral and big enough to store tag2");
  static constexpr size_t TAG2_MASK = (1ULL << TAG2_BITS) - 1;
  static constexpr size_t VALUE_MASK = ((1ULL << VALUE_BITS) - 1) & ~TAG2_MASK;
};

template <typename T, typename T2, size_t alignment>
DoubleTagPtr<T, T2, alignment>::DoubleTagPtr() noexcept
  : raw_value()
{
}

template <typename T, typename T2, size_t alignment>
DoubleTagPtr<T, T2, alignment>::DoubleTagPtr(uint64_t raw) noexcept
  : raw_value(raw)
{
}

template <typename T, typename T2, size_t alignment>
DoubleTagPtr<T, T2, alignment>::DoubleTagPtr(Value value) noexcept
  : raw_value(reinterpret_cast<uint64_t>(value))
{
  assert(this->value() == value);
}

template <typename T, typename T2, size_t alignment>
DoubleTagPtr<T, T2, alignment>::DoubleTagPtr(Value value, Tag1 tag1, Tag2 tag2)
    noexcept
  : raw_value(*reinterpret_cast<RawType*>(&value) |
              (static_cast<RawType>(tag1) << VALUE_BITS) | static_cast<Tag2>(tag2))
{
  assert(this->value() == value && this->tag1() == tag1 &&
         this->tag2() == tag2);
}

template <typename T, typename T2, size_t alignment>
typename DoubleTagPtr<T, T2, alignment>::Value
DoubleTagPtr<T, T2, alignment>::value() const noexcept
{
  uint64_t tmp = raw_value & VALUE_MASK;
  return *reinterpret_cast<Value*>(&tmp);
}

template <typename T, typename T2, size_t alignment>
typename DoubleTagPtr<T, T2, alignment>::Tag1
DoubleTagPtr<T, T2, alignment>::tag1() const noexcept
{
  return static_cast<Tag1>(raw_value >> VALUE_BITS);
}

template <typename T, typename T2, size_t alignment>
typename DoubleTagPtr<T, T2, alignment>::Tag2
DoubleTagPtr<T, T2, alignment>::tag2() const noexcept
{
  return static_cast<Tag2>(raw_value & TAG2_MASK);
}

template <typename T, typename T2, size_t alignment>
bool
DoubleTagPtr<T, T2, alignment>::operator==(DoubleTagPtr other) const noexcept
{
  return raw_value == other.raw_value;
}

template <typename T, typename T2, size_t alignment>
bool
DoubleTagPtr<T, T2, alignment>::operator!=(DoubleTagPtr other) const noexcept
{
  return !(*this == other);
}


/// \brief Provides atomic access to tagged pointers.
/// The tag usually serves as an ABA counter in concurrent data structures.
template <typename T>
class AtomicTaggedPtr
{
public:
  using TaggedPtr = T;

  AtomicTaggedPtr();
  constexpr explicit AtomicTaggedPtr(TaggedPtr tagged_ptr);

  TaggedPtr operator=(TaggedPtr tagged_ptr);
  TaggedPtr operator=(TaggedPtr tagged_ptr) volatile;

  bool is_lock_free() const;
  bool is_lock_free() const volatile;

  void store(TaggedPtr desired,
             std::memory_order order = std::memory_order_seq_cst);
  void store(TaggedPtr desired,
             std::memory_order order = std::memory_order_seq_cst) volatile;

  TaggedPtr load(std::memory_order order = std::memory_order_seq_cst) const;
  TaggedPtr load(std::memory_order order = std::memory_order_seq_cst) const
      volatile;

  operator TaggedPtr() const;
  operator TaggedPtr() const volatile;

  TaggedPtr exchange(TaggedPtr desired,
                      std::memory_order order = std::memory_order_seq_cst);
  TaggedPtr exchange(TaggedPtr desired,
                      std::memory_order order = std::memory_order_seq_cst)
      volatile;

  bool compare_exchange_weak(TaggedPtr& expected, TaggedPtr desired,
      std::memory_order success, std::memory_order failure);
  bool compare_exchange_weak(TaggedPtr& expected, TaggedPtr desired,
      std::memory_order success, std::memory_order failure) volatile;

  bool compare_exchange_weak(TaggedPtr& expected, TaggedPtr desired,
      std::memory_order order = std::memory_order_seq_cst);
  bool compare_exchange_weak(TaggedPtr& expected, TaggedPtr desired,
      std::memory_order order = std::memory_order_seq_cst) volatile;

  bool compare_exchange_strong(TaggedPtr& expected, TaggedPtr desired,
      std::memory_order success, std::memory_order failure);
  bool compare_exchange_strong(TaggedPtr& expected, TaggedPtr desired,
      std::memory_order success, std::memory_order failure) volatile;

  bool compare_exchange_strong(TaggedPtr& expected, TaggedPtr desired,
      std::memory_order order = std::memory_order_seq_cst);
  bool compare_exchange_strong(TaggedPtr& expected, TaggedPtr desired,
      std::memory_order order = std::memory_order_seq_cst) volatile;

  bool operator==(TaggedPtr other) const noexcept;
  bool operator!=(TaggedPtr other) const noexcept;

private:
  std::atomic<typename T::RawType> m_raw_value;
};

template <typename T>
AtomicTaggedPtr<T>::AtomicTaggedPtr()
  : m_raw_value()
{
}

template <typename T>
constexpr
AtomicTaggedPtr<T>::AtomicTaggedPtr(TaggedPtr tagged_ptr)
  : m_raw_value(tagged_ptr.raw_value)
{
}

template <typename T>
typename AtomicTaggedPtr<T>::TaggedPtr
AtomicTaggedPtr<T>::operator=(TaggedPtr tagged_ptr)
{
  m_raw_value = tagged_ptr.raw_value;
}

template <typename T>
typename AtomicTaggedPtr<T>::TaggedPtr
AtomicTaggedPtr<T>::operator=(TaggedPtr tagged_ptr) volatile
{
  m_raw_value = tagged_ptr.raw_value;
}

template <typename T>
bool
AtomicTaggedPtr<T>::is_lock_free() const
{
  return m_raw_value.is_lock_free();
}

template <typename T>
bool
AtomicTaggedPtr<T>::is_lock_free() const volatile
{
  return m_raw_value.is_lock_free();
}

template <typename T>
void
AtomicTaggedPtr<T>::store(TaggedPtr desired, std::memory_order order)
{
  return m_raw_value.store(desired.raw_value, order);
}

template <typename T>
void
AtomicTaggedPtr<T>::store(TaggedPtr desired, std::memory_order order) volatile
{
  return m_raw_value.store(desired.raw_value, order);
}

template <typename T>
typename AtomicTaggedPtr<T>::TaggedPtr
AtomicTaggedPtr<T>::load(std::memory_order order) const
{
  return TaggedPtr(m_raw_value.load(order));
}

template <typename T>
typename AtomicTaggedPtr<T>::TaggedPtr
AtomicTaggedPtr<T>::load(std::memory_order order) const volatile
{
  return TaggedPtr(m_raw_value.load(order));
}

template <typename T>
AtomicTaggedPtr<T>::operator TaggedPtr() const
{
  return load();
}

template <typename T>
AtomicTaggedPtr<T>::operator TaggedPtr() const volatile
{
  return load();
}

template <typename T>
typename AtomicTaggedPtr<T>::TaggedPtr
AtomicTaggedPtr<T>::exchange(TaggedPtr desired, std::memory_order order)
{
  return TaggedPtr(m_raw_value.exchange(desired.raw_value, order));
}

template <typename T>
typename AtomicTaggedPtr<T>::TaggedPtr
AtomicTaggedPtr<T>::exchange(TaggedPtr desired, std::memory_order order)
    volatile
{
  return TaggedPtr(m_raw_value.exchange(desired.raw_value, order));
}

template <typename T>
bool
AtomicTaggedPtr<T>::compare_exchange_weak(TaggedPtr& expected,
    TaggedPtr desired, std::memory_order success, std::memory_order failure)
{
  return m_raw_value.compare_exchange_weak(expected.raw_value,
       desired.raw_value, success, failure);
}

template <typename T>
bool
AtomicTaggedPtr<T>::compare_exchange_weak(TaggedPtr& expected,
    TaggedPtr desired, std::memory_order success, std::memory_order failure)
    volatile
{
  return m_raw_value.compare_exchange_weak(expected.raw_value,
       desired.raw_value, success, failure);
}

template <typename T>
bool
AtomicTaggedPtr<T>::compare_exchange_weak(TaggedPtr& expected,
    TaggedPtr desired, std::memory_order order)
{
  return m_raw_value.compare_exchange_weak(expected.raw_value,
      desired.raw_value, order);
}

template <typename T>
bool
AtomicTaggedPtr<T>::compare_exchange_weak(TaggedPtr& expected,
    TaggedPtr desired, std::memory_order order) volatile
{
  return m_raw_value.compare_exchange_weak(expected.raw_value,
      desired.raw_value, order);
}

template <typename T>
bool
AtomicTaggedPtr<T>::compare_exchange_strong(TaggedPtr& expected,
    TaggedPtr desired, std::memory_order success, std::memory_order failure)
{
  return m_raw_value.compare_exchange_strong(expected.raw_value,
      desired.raw_value, success, failure);
}

template <typename T>
bool
AtomicTaggedPtr<T>::compare_exchange_strong(TaggedPtr& expected,
    TaggedPtr desired, std::memory_order success, std::memory_order failure)
    volatile
{
  return m_raw_value.compare_exchange_strong(expected.raw_value,
      desired.raw_value, success, failure);
}

template <typename T>
bool
AtomicTaggedPtr<T>::compare_exchange_strong(TaggedPtr& expected,
    TaggedPtr desired, std::memory_order order)
{
  return m_raw_value.compare_exchange_strong(expected.raw_value,
      desired.raw_value, order);
}

template <typename T>
bool
AtomicTaggedPtr<T>::compare_exchange_strong(TaggedPtr& expected,
    TaggedPtr desired, std::memory_order order) volatile
{
  return m_raw_value.compare_exchange_strong(expected.raw_value,
      desired.raw_value, order);
}

template <typename T>
bool
AtomicTaggedPtr<T>::operator==(TaggedPtr other) const noexcept
{
  return load() == other;
}

template <typename T>
bool
AtomicTaggedPtr<T>::operator!=(TaggedPtr other) const noexcept
{
  return !(*this == other);
}

} // namespace ni
