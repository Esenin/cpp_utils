#ifndef THREADSAFE_HASHMAP_CONCURRENT_MAP_TEST_H
#define THREADSAFE_HASHMAP_CONCURRENT_MAP_TEST_H

#include <vector>

#include "../include/threadsafe_hashmap.h"

using my_concurrency::ThreadsafeHashmap;

namespace tests {

#ifdef NDEBUG
#undef NDEBUG
#define RESTORE_NDEBUG
#endif

class ConcurrentMapTest {
 public:
  void TestAll() {
    SimpleTests();
    ManyOperationsTest();
    ResizeTest();
    ParallelInsert();


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
  }

  void ManyOperationsTest() {
    Map map;
    for (int x : keys)
      map.Insert(x, x * 10);

    assert(keys.size() == map.Size());
    for (int x : keys)
      assert(make_pair(true, x * 10) == map.Lookup(x));

    for (int i = 0; i < keys.size() / 2; i++)
      assert(Map::kOperationSuccess == map.Remove(keys[i]));

    assert(keys.size() - keys.size() / 2 == map.Size());
    for (int i = 0; i < keys.size(); i++)
      assert(make_pair(i >= keys.size() / 2, i < keys.size() / 2? 0 :  keys[i] * 10) == map.Lookup(keys[i]));

    map.Clear();
    map.Insert(1, 10);
    assert(make_pair(true, 10) == map.Lookup(1));
  }

  void ResizeTest() {
    int num_buckets = 50; // resize on load factor 0.75
    Map map(num_buckets);

    int data_size = 1000;
    for (int i = 0; i < data_size; i++)
      map.Insert(i, i * 10);

    for (int i = 0; i < data_size; i++)
      assert(make_pair(true, i * 10) == map.Lookup(i));
  }

  void ParallelInsert() {
    Map map(1000);
    int chunk_size = 100;
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
  }




};

#ifdef RESTORE_NDEBUG
#undef RESTORE_NDEBUG
#define NDEBUG
#endif

} // namespace tests

#endif //THREADSAFE_HASHMAP_CONCURRENT_MAP_TEST_H
