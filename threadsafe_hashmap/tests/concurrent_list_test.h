#ifndef THREADSAFE_HASHMAP_CONCURRENT_LIST_TEST_H
#define THREADSAFE_HASHMAP_CONCURRENT_LIST_TEST_H

#include <assert.h>
#include <numeric>
#include <iostream>
#include <thread>
#include <vector>
#include <future>

#include "../src/concurrent_linked_list.h"

using my_concurrency::internals::ConcurrentLinkedList;


namespace tests {

#ifdef NDEBUG
#undef NDEBUG
#define RESTORE_NDEBUG
#endif

class ConcurrentListTest {
 public:
  ConcurrentListTest() : keys{1, 2, 5, 7, 11, 13, 17, 19} {}

  void TestAll() {
    ManyReadersTest();
    WriterReaderTest();
    ManyWriters();
    SerialLockReleaseTest();

    std::cout << "Concurrent part of the linked list tests passed." << std::endl;
  }

 private:
  void LoadList(ConcurrentLinkedList<int,int> &list) {
    for (int x : keys)
      list.Insert(x, x * 10);
  }

  int ListSumSerialized(ConcurrentLinkedList<int, int> &list) {
    return std::accumulate(list.Begin(), list.End(), 0, [] (int acc, auto key_value) { return acc + key_value.second; });
  }

  std::vector<int> keys;


  void ManyReadersTest() {
    ConcurrentLinkedList<int, int> list;
    LoadList(list);

    auto sum_list = [&list] () {  // the answer is  750
      int sum = 0;
      for (int i = 0; i < 20; i++) {
        auto result = list.Lookup(i);
        sum += result.first? result.second : 0;
      }
      return sum;
    };


    std::vector<std::future<int>> results;
    for (int i = 0; i < std::thread::hardware_concurrency(); i++) {
      results.push_back(std::async(std::launch::async, sum_list));
    }

    for (auto &result : results)
      assert(750 == result.get());
  }

  void WriterReaderTest() {
    ConcurrentLinkedList<int, int> list;
    int num_elements = 101;

    auto writer = [&list, num_elements] () {
      for (int i = 0; i < num_elements; i++)
        list.Insert(i, i);
    };

    auto sum_reader = [&list, num_elements] () -> int {
      int counter = 0;
      int sum = 0;
      while (counter < num_elements) {
        for (int i = 0; i < num_elements; i++) {
          auto result = list.Lookup(i);
          if (result.first) {
            sum += result.second;
            counter++;
          }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
      return sum;
    };

    std::thread writer_thread(writer);
    auto result = std::async(std::launch::async, sum_reader);
    writer_thread.join();
    assert(5050 == result.get());  // 5050 = sum(1 +... + 100)
  }

  void ManyWriters() {
    ConcurrentLinkedList<int, int> list;
    int num_elements = 101;

    auto writer = [&list, num_elements] (bool do_write_even) {
      for (int i = do_write_even? 0 : 1; i < num_elements; i += 2)
        list.Insert(i, i);
    };

    std::thread odd_writer(writer, false);
    std::thread even_writer(writer, true);

    odd_writer.join();
    even_writer.join();

    assert(5050 == ListSumSerialized(list));
  }

  void SerialLockReleaseTest() {
    ConcurrentLinkedList<int, int> list;

    int num_elements = 101;

    auto writer = [&list, num_elements] () {
      for (int i = 0; i < num_elements; i++)
        list.Insert(i, i);
    };

    list.Insert(0, 0);
    auto iter = list.Begin(); // locks the entire list

    std::thread writer_thread(writer);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    assert(1 == list.Size());

    while (++iter != list.End()) {} // should release the lock
    ++iter; // check for double-unlock behaviour
    ++iter;

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    assert(list.Size() > 1);

    writer_thread.join();

    assert(5050 == ListSumSerialized(list));
  }
};


#ifdef RESTORE_NDEBUG
#undef RESTORE_NDEBUG
#define NDEBUG
#endif

} // namespace tests
#endif //THREADSAFE_HASHMAP_CONCURRENT_LIST_TEST_H
