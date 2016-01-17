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
/// \param T type of the elements
template <template <typename T> typename Impl, typename T>
class Queue
{
public:
  using Self = Impl<T>;
  using Element = T;

  template <typename U>
  bool put(U&& element);

  bool get(Element* element);
};

template <template <typename T> typename Impl, typename T>
template <typename U>
bool Queue<Impl, T>::put(U&& element)
{
  return static_cast<Self*>(this)->push(std::forward<U>(element));
}

template <template <typename T> typename Impl, typename T>
bool Queue<Impl, T>::get(Element* element)
{
  return static_cast<Self*>(this)->pop(element);
}

} // namespace ni
