#ifndef THREADSAFE_HASHMAP_CONCURRENT_LIST_TEST_H
#define THREADSAFE_HASHMAP_CONCURRENT_LIST_TEST_H

#ifdef NDEBUG
#undef NDEBUG
  #define RESTORE_NDEBUG
#endif

#include <assert.h>

#ifdef RESTORE_NDEBUG
#undef RESTORE_NDEBUG
  #define NDEBUG
#endif

#include <numeric>
#include <iostream>
#include <thread>
#include <vector>
#include <future>

#include "../src/bucket.h"
#include "hashmap_test.h"

using my_concurrency::internals::Bucket;


namespace tests {

class ConcurrentListTest {
 public:
  ConcurrentListTest() : keys{1, 2, 5, 7, 11, 13, 17, 19} {}

  void TestAll() {
    ManyReadersTest();
    WriterReaderTest();
    ManyWriters();
    ExclusiveIteratorTest();

    std::cout << "Concurrent part of the linked list tests passed." << std::endl;
  }

 private:
  void LoadList(Bucket<int,int> &list) {
    for (int x : keys)
      list.Insert(x, x * 10);
  }

  std::vector<int> keys;


  void ManyReadersTest() {
    Bucket<int, int> list;
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
    for (uint32_t i = 0; i < std::thread::hardware_concurrency(); i++) {
      results.push_back(std::async(std::launch::async, sum_list));
    }

    for (auto &result : results)
      assert(750 == result.get());
    std::cout << "\t" << __func__ << " passed" << std::endl;
  }

  void WriterReaderTest() {
    Bucket<int, int> list;
    int num_elements = 201;

    auto writer = [&list, num_elements] () {
      for (int i = 0; i < num_elements; i++)
        list.Insert(i, i);

    };

    auto sum_reader = [&list, num_elements] () -> int {
      int sum = 0;
      int cur_idx = 0;
      while (cur_idx < num_elements) {
        auto result = list.Lookup(cur_idx);

        while (!result.first)
          result = list.Lookup(cur_idx);

        sum += result.second;
        cur_idx++;
      }
      return sum;
    };

    std::thread writer_thread(writer);
    auto result = std::async(std::launch::async, sum_reader);
    writer_thread.join();
    assert(20100 == result.get());  // 20100 = sum(1 +... + 200)
    std::cout << "\t" << __func__ << " passed" << std::endl;
  }

  void ManyWriters() {
    Bucket<int, int> list;
    int num_elements = 101;

    auto writer = [&list, num_elements] (bool do_write_even) {
      for (int i = do_write_even? 0 : 1; i < num_elements; i += 2)
        list.Insert(i, i);
    };

    std::thread odd_writer(writer, false);
    std::thread even_writer(writer, true);

    odd_writer.join();
    even_writer.join();

    assert(5050 == std::accumulate(list.BeginSync(), list.End(), 0,
                                   [] (int acc, auto key_value) { return acc + key_value.second; }));
    std::cout << "\t" << __func__ << " passed" << std::endl;
  }

  void ExclusiveIteratorTest() {
    Bucket<int, int> list;
    list.Insert(10, 100);
    list.Insert(20, 200);
    list.Insert(30, 300);

    auto iter = list.BeginSync();

    assert((*iter).first * 10 == (*iter).second);
    ++iter;

    std::thread writer([&]() { this->LoadList(list); });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    assert(3 == list.Size());

    assert((*iter).first * 10 == (*iter).second);
    ++iter;
    assert((*iter).first * 10 == (*iter).second);

    ++iter; // mutex unlock
    writer.join();

    assert(keys.size() + 3 == list.Size());

  }

};

} // namespace tests
#endif //THREADSAFE_HASHMAP_CONCURRENT_LIST_TEST_H
