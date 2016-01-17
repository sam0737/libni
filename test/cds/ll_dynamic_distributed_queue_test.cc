#include <thread>

#include <ni/cds/ms_queue.hh>
#include <ni/cds/distributed/dynamic.hh>
#include <catch.hpp>

using namespace ni;

TEST_CASE("LLDynamicDistributedMSQueue-FIFO")
{
  using Queue = LLDynamicDistributed<MSQueue<int>>;
  Queue queue(64);

  std::vector<std::thread> threads;

  threads.emplace_back([&]
                       {
                         Queue::BackendPtr local_backend;
                         for (int i = 0; i < 1000; ++i)
                         {
                           queue.put(local_backend, i);
                         }
                         queue.deregister_thread(local_backend);
                       });

  threads.emplace_back(
    [&]
    {
      Queue::BackendPtr local_backend;
      for (int i = 0; i < 1000; ++i)
      {
        int value;
        while (!queue.get(local_backend, &value))
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        REQUIRE(value == i);
      }
      queue.deregister_thread(local_backend);
    });

  for (auto& t : threads)
    t.join();
}
