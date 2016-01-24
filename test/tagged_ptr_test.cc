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
#include <catch.hpp>

#include <ni/mpl/unit.hh>
#include <ni/tagged_ptr.hh>

using namespace ni;

TEST_CASE("TaggedPtr")
{
  Unit obj;
  Unit* ptr = &obj;

  REQUIRE(TaggedPtr<Unit>(ptr, 0).value() == ptr);
  REQUIRE(TaggedPtr<Unit>(ptr, 42).value() == ptr);
  REQUIRE(TaggedPtr<Unit>(ptr, 42).tag() == 42);
}

TEST_CASE("DoubleTagPtr")
{
  struct alignas(16) Type
  {
    int i;
  };
  using Ptr = DoubleTagPtr<Type, int>;

  Type tmp{7};

  REQUIRE(Ptr(&tmp, 42, 3).value() == &tmp);
  REQUIRE(Ptr(&tmp, 42, 3).tag1() == 42);
  REQUIRE(Ptr(&tmp, 0, 3).tag2() == 3);
}

TEST_CASE("AtomicTaggedValue<TaggedPtr>")
{
  using Ptr = TaggedPtr<Unit>;
  using AtomicPtr = AtomicTaggedPtr<Ptr>;

  Unit obj;
  Ptr p(&obj, 7);
  AtomicPtr ap(p);

  REQUIRE(ap.load(std::memory_order_relaxed) == p);
  ap.store(Ptr(nullptr, 42), std::memory_order_relaxed);
  REQUIRE(ap == Ptr(nullptr, 42));
}
