#include <ni/hash/multi_linear_hash.hh>
#include <catch.hpp>

using namespace ni;

TEST_CASE("MultiLinearDoubleHash")
{
  MultiLinearDoubleHash<64> hasher(42);

  REQUIRE(hasher("Strongly universal string hashing is fast") == 0xfa787257);
}
