#ifndef THREADSAFE_HASHMAP_CONCURRENT_MAP_TEST_H
#define THREADSAFE_HASHMAP_CONCURRENT_MAP_TEST_H

#ifdef NDEBUG
#undef NDEBUG
  #define RESTORE_NDEBUG
#endif

#include <assert.h>

#ifdef RESTORE_NDEBUG
#undef RESTORE_NDEBUG
  #define NDEBUG
#endif

#include <chrono>
#include <random>
#include <set>
#include <vector>

#include "../include/threadsafe_hashmap.h"

using my_concurrency::ThreadsafeHashmap;

namespace tests {

class ConcurrentMapTest {
 public:
  void TestAll() {
    SimpleTests();
    GenericTest();
    ManyOperationsTest();
    ResizeTest();
    ResizeOverwriteTest();
    ParallelInsert();
    ParallelResizeTest();
    ConcurrentWriteRemoveTest();
//    HighLoadTest();

    std::cout << "Concurrent Hashmap tests passed." << std::endl;
  }

 private:
  std::vector<int> keys{1, 2, 5, 7, 11, 13, 17, 19, 20};

  typedef ThreadsafeHashmap<int, int> Map;

  void SimpleTests() {
    Map map;
    assert(map.Empty());
    map.Insert(1, 10);
    assert(1 == map.Size());
    assert(!map.Empty());
    assert(make_pair(true, 10) == map.Lookup(1));
    map.Insert(1, 11);
    assert(1 == map.Size());
    assert(make_pair(true, 11) == map.Lookup(1));
    assert(Map::kOperationSuccess == map.Remove(1));
    assert(Map::kOperationFailed == map.Remove(1));
    assert(0 == map.Size() && map.Empty());
    map.Clear();

    for (int i = 0; i < 10; i++)
      map.Insert(i, i * 10);

    Map copied = map;
    copied = map;

    for (int i = 0; i < 10; i++)
      assert(make_pair(true, i * 10) == copied.Lookup(i));
    map.Clear();
    assert(10 == copied.Size());
    std::cout << "\t" << __func__ << " passed" << std::endl;
  }

  void GenericTest() {
    struct SomeStruct {
      std::vector<double> values;
      bool operator==(const SomeStruct &rhs) const {
        return values == rhs.values;
      }
    };

    SomeStruct x;
    x.values.push_back(1);
    SomeStruct y;
    x.values.push_back(2);
    ThreadsafeHashmap<int, SomeStruct> map_to;
    map_to.Insert(1, x);
    map_to.Insert(2, y);
    assert(make_pair(true, x) == map_to.Lookup(1));

    auto custom_hash = [] (const SomeStruct &t) {
      return t.values.size() == 0? 0ull : (uint64_t) t.values.front();
    };

    ThreadsafeHashmap<SomeStruct, SomeStruct> map(64, custom_hash);
    map.Insert(x, y);
    map.Insert(y, x);
    assert(make_pair(true, y) == map.Lookup(x));
  }

  void ManyOperationsTest() {
    Map map;
    for (int x : keys)
      map.Insert(x, x * 10);

    assert(keys.size() == map.Size());
    for (int x : keys)
      assert(make_pair(true, x * 10) == map.Lookup(x));

    for (uint64_t i = 0; i < keys.size() / 2; i++)
      assert(Map::kOperationSuccess == map.Remove(keys[i]));

    assert(keys.size() - keys.size() / 2 == map.Size());
    for (uint64_t i = 0; i < keys.size(); i++)
      assert(make_pair(i >= keys.size() / 2, i < keys.size() / 2? 0 :  keys[i] * 10) == map.Lookup(keys[i]));

    map.Clear();
    map.Insert(1, 10);
    assert(make_pair(true, 10) == map.Lookup(1));
    std::cout << "\t" << __func__ << " passed" << std::endl;
  }

  void ResizeTest() {
    int num_buckets = 50; // resize on load factor 0.75
    Map map(num_buckets);

    int data_size = 1000;
    for (int i = 0; i < data_size; i++)
      map.Insert(i, i * 10);

    for (int i = 0; i < data_size; i++)
      assert(make_pair(true, i * 10) == map.Lookup(i));
    std::cout << "\t" << __func__ << " passed" << std::endl;
  }

  void ResizeOverwriteTest() {
    int num_buckets = 100;
    Map map(num_buckets);
    int key = 0;
    for (; key < Map::kMaxLoadFactor * num_buckets + 1; key++)
      map.Insert(key, 1);
    // now the map is in resizing state
    // The inserted values are in old map. Now we are writing to the new one
    map.Insert(50, 999);
    map.Insert(51, 999);
    map.Insert(60, 999);

    // continue to fill the table in purpose to close resizing state
    for (int i = 0; i < 15; i++)
      map.Insert(key + i, 1);

    for (int i = 0; i < key + 15; i++)
      assert(make_pair(true, (i == 50 || i == 51 || i == 60)? 999 : 1) == map.Lookup(i));


    std::cout << "\t" << __func__ << " passed" << std::endl;
  }

  void ParallelInsert() {
    int num_buckets = 1000;
    Map map(num_buckets);
    int chunk_size = 200;

    assert(3 * chunk_size < 0.75 * num_buckets); // This test doesn't include resizing. Next ones does

    auto writer = [&map, chunk_size] (int start_value) {
      for (int i = start_value; i < start_value + chunk_size; i++)
        map.Insert(i, i * 10);
    };

    std::thread t1(writer, 0);
    std::thread t2(writer, chunk_size);
    std::thread t3(writer, 2 * chunk_size);

    t1.join();
    t2.join();
    t3.join();

    for (int i = 0; i < 3 * chunk_size; i++)
      assert(make_pair(true, i * 10) == map.Lookup(i));

    map.Clear();
    int start_value = 123456;
    t1 = std::thread(writer, start_value);
    t2 = std::thread(writer, start_value);
    t3 = std::thread(writer, start_value);
    t1.join();
    t2.join();
    t3.join();

    assert(chunk_size == (int)map.Size());
    for (int i = start_value; i < start_value + chunk_size; i++)
      assert(make_pair(true, i * 10) == map.Lookup(i));
    std::cout << "\t" << __func__ << " passed" << std::endl;
  }

  void ParallelResizeTest() {
    Map map(20);
    int chunk_size = 1000;
    auto writer = [&map, chunk_size] (int start_value) {
      for (int i = start_value; i < start_value + chunk_size; i++)
        map.Insert(i, i * 10);
    };
    std::thread t1(writer, 0);
    std::thread t2(writer, chunk_size);
    std::thread t3(writer, 2 * chunk_size);

    t1.join();
    t2.join();
    t3.join();

    for (int i = 0; i < 3 * chunk_size; i++)
      assert(make_pair(true, i * 10) == map.Lookup(i));
    assert(3 * chunk_size == (int)map.Size());
    std::cout << "\t" << __func__ << " passed" << std::endl;
  }

  void ConcurrentWriteRemoveTest() {
    Map map;
    int data_size = 10001;
    auto writer = [&map, data_size] () {
      for (int i = 0; i < data_size; i++)
        map.Insert(i, i * 10);
    };

    auto remove_even = [&map, data_size] () {
      int counter = 0;
      while (counter < data_size / 2) {
        for (int i = 0; i < data_size; i += 2)
          counter += map.Remove(i)? 1 : 0;
      }
    };
    std::thread t1(writer);
    std::thread t2(remove_even);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    assert(map.Size() > 0);

    t1.join();
    t2.join();
    for (int i = 0; i < data_size; i++)
      assert(make_pair((i & 1) == 1, (i & 1) == 1? i * 10 : 0) == map.Lookup(i));

    assert(data_size / 2 == (int)map.Size());
    std::cout << "\t" << __func__ << " passed" << std::endl;
  }

  void HighLoadTest() {
    const int kHwThreads = std::thread::hardware_concurrency();
    if (kHwThreads < 3)
      return;
    std::cout << "\t> " << __func__ << " is in progress... (takes some time, ~ 1 min)" << std::endl;

    Map map;
    std::default_random_engine generator(1);
    std::normal_distribution<double> distribution(0, 1*1000000);
    const uint64_t kDataSize = 5 * 1024 * 1024 / sizeof(int); // 5 MB
    std::vector<int> data(kDataSize, 0);
    for (uint64_t i = 0; i < kDataSize; i++)
      data[i] = static_cast<int>(distribution(generator));
    std::set<int> uniq_data(data.begin(), data.end());

    auto writer = [&data, &map] (int start_idx) {
      for (uint64_t counter = start_idx; counter < data.size(); counter++) {
        uint64_t idx = counter % data.size();
        map.Insert(data[idx], data[idx] * 10);
      }
    };

    auto consumer = [&map, &uniq_data] (bool remove, bool check_ones)  {
      for (auto key : uniq_data) {
        while (false == map.Lookup(key).first) {
          assert(!check_ones);
          std::this_thread::yield();
        }

        assert(key * 10 == map.Lookup(key).second);
        if (remove)
          assert(map.Remove(key));
      }
    };

    std::cout << "\t\t..."  << std::endl;

    std::vector<std::thread> threads;

    for (int i = 0; i < kHwThreads - 1; i++)
      threads.push_back(std::thread(writer, i * kDataSize / (kHwThreads - 1)));

    std::thread reader = std::thread(consumer, false, false);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    assert(map.Size() > 0);

    for (auto &thread: threads) {
      thread.join();
      std::cout << "\t\twriter thread finished" << std::endl;
    }

    reader.join();
    std::cout << "\t\treader thread finished" << std::endl;

    consumer(true, true);
    assert(0 == map.Size());

    std::cout << "\t\t..."  << std::endl;

    for (int i = 0; i < kHwThreads - 1; i++)
      threads[i] = std::thread(writer, i * kDataSize / (kHwThreads - 1));
    consumer(true, false);

    for (auto &thread: threads)
      thread.join();

    map.Clear();
    assert(0 == map.Size());
    std::cout << "\t" << __func__ << " passed" << std::endl;
  }



};

} // namespace tests

#endif //THREADSAFE_HASHMAP_CONCURRENT_MAP_TEST_H
