#include <ni/mpl/unit.hh>
#include <ni/tagged_ptr.hh>
#include <catch.hpp>

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
