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
#include <array>
//#include <cstring>
//#include <type_traits>

#include <ni/random.hh>
#include <ni/string_view.hh>

namespace ni
{

/// \brief A family of strongly universal string hashing algorithms
/// \param l Maximum length of bytes it needs to hash
/// \param Rng The PRNG used internally
///
/// **Reference**
/// * Owen Kaser and Daniel Lemire, Strongly universal string hashing is fast,
/// Computer Journal (2014) 57 (11): 1624-1638. http://arxiv.org/abs/1202.4961
template <size_t l, typename Rng = pcg64>
class MultiLinearDoubleHash
{
  static_assert(std::is_same<typename Rng::result_type, uint64_t>::value,
                "result_type of the PRNG used must be uint64_t");

public:
  static constexpr size_t MAX_LEN = l;

  MultiLinearDoubleHash();
  explicit MultiLinearDoubleHash(uint64_t seed);

  uint32_t operator()(const uint8_t* bytes, size_t length) const noexcept;
  uint32_t operator()(const string_view str) const noexcept;
  uint32_t operator()(const char* str, size_t length) const noexcept;

private:
  std::array<uint64_t, l + 2> m_rand;

  void fill_random_data(Rng& rng);
};

template <size_t l, typename Rng>
MultiLinearDoubleHash<l, Rng>::MultiLinearDoubleHash()
  : m_rand()
{
  pcg_extras::seed_seq_from<std::random_device> seed_source;
  Rng rng(seed_source);
  fill_random_data(rng);
}

template <size_t l, typename Rng>
MultiLinearDoubleHash<l, Rng>::MultiLinearDoubleHash(uint64_t seed)
  : m_rand()
{
  Rng rng(seed);
  fill_random_data(rng);
}

template <size_t l, typename Rng>
uint32_t
MultiLinearDoubleHash<l, Rng>::operator()(const uint8_t* bytes, size_t length)
    const noexcept
{
  assert(bytes);
  assert(length <= MAX_LEN);

  const uint64_t* random = m_rand.data();
  uint64_t sum = *random++;
  const uint8_t* const end = bytes + length;

  if (length & 1)
  {
    while (bytes != end)
    {
      sum += *random++ * *bytes++;
    }
    sum += *random;
  }
  else
  {
    uint64_t s2 = 0;

    while (bytes != end)
    {
      sum += *random++ * *bytes++;
      s2 += *random++ * *bytes++;
    }
    sum += *random + s2;
  }
  return static_cast<uint32_t>(sum >> 32);
}

template <size_t l, typename Rng>
uint32_t
MultiLinearDoubleHash<l, Rng>::operator()(const string_view str) const noexcept
{
  return this->operator()(reinterpret_cast<const uint8_t*>(str.data()),
                          str.size());
}

template <size_t l, typename Rng>
uint32_t
MultiLinearDoubleHash<l, Rng>::operator()(const char* str, size_t length) const
    noexcept
{
  return this->operator()(reinterpret_cast<const uint8_t*>(str), length);
}

template <size_t l, typename Rng>
void
MultiLinearDoubleHash<l, Rng>::fill_random_data(Rng& rng)
{
  std::uniform_int_distribution<std::uint64_t> uniform_dist(0, UINT64_MAX);

  for (size_t i = 0, end = m_rand.size(); i < end; ++i)
  {
    m_rand[i] = uniform_dist(rng);
  }
}

} // namespace ni
