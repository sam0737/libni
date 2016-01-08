#include <thread>

#include <ni/cds/ms_queue.hh>
#include <catch.hpp>

using namespace ni;

TEST_CASE("MSQueue-FIFO")
{
  MSQueue<int> queue;
  REQUIRE(queue.empty());

  std::vector<std::thread> threads;

  threads.emplace_back([&]
  {
    for (int i = 0; i < 1000; ++i)
    {
      queue.push(i);
    }
  });

  threads.emplace_back([&]
  {
    for (int i = 0; i < 1000; ++i)
    {
      int value;
      while (!queue.pop(&value))
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      REQUIRE(value == i);
    }
  });

  for (auto& t: threads)
    t.join();

  REQUIRE(queue.empty());
}

TEST_CASE("MSQueue-ABACounter")
{
  MSQueue<int> queue;
  REQUIRE(queue.empty());

  std::atomic<int> in(0);
  std::vector<std::thread> threads;

  for (int i = 0; i < 3; ++i)
  {
    threads.emplace_back([&]
    {
      for (int i = 0; i < 333; ++i)
      {
        queue.push(in.fetch_add(1, std::memory_order_relaxed));
      }
    });
  }

  threads.emplace_back([&]
  {
    int sum = 0;
    for (int i = 0; i < 999; ++i)
    {
      int value;
      uint16_t state;
      while (!queue.pop(&value, &state))
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      sum += value;
      REQUIRE(state == i);
    }
    REQUIRE(sum == (0 + 998) * 999 / 2);
  });

  for (auto& t: threads)
    t.join();

  REQUIRE(queue.empty());
}
