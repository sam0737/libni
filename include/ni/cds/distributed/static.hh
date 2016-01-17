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

namespace ni
{

/// \brief Common interface for distributed data structures
template <typename T, typename B>
class DistributedInterface
{
public:
  using Element = typename T::Element;
  using Backend = T;
  using Balancer = B;
  using BalancerPtr = std::unique_ptr<Balancer>;

  DistributedInterface(BalancerPtr&& balancer);

  template <typename U>
  void put(U&& element);
  void get(Element* element);

private:
  BalancerPtr m_balancer;
};

template <typename T, typename B>
DistributedInterface<T, B>::DistributedInterface(BalancerPtr&& balancer)
  : m_balancer(std::move(balancer))
{
}

template <typename T, typename B>
template <typename U>
void DistributedInterface<T, B>::put(U&& element)
{
}

template <typename T, typename B>
void DistributedInterface<T, B>::get(Element* element)
{
}

} // namespace ni
