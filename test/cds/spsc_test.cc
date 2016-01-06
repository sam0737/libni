#include <thread>

#include <ni/cds/spsc_buffer_list.hh>
#include "../catch.hh"

using namespace ni;

TEST_CASE("SPSCRingBuffer")
{
  SPSCRingBuffer<int, -1> ringbuf(8);
  REQUIRE(ringbuf.empty());
  REQUIRE(ringbuf.available());
  REQUIRE(ringbuf.len() == 0);

  std::thread t1([&]
  {
    for (int i = 0; i < 100; ++i)
    {
      while (!ringbuf.push(i))
        pthread_yield();
    }
  });

  std::thread t2([&]
  {
    for (int i = 0; i < 100; ++i)
    {
      int val;
      while ((val = ringbuf.pop()) == -1)
        pthread_yield();
      REQUIRE(val == i);
    }
  });

  t1.join();
  t2.join();
}
