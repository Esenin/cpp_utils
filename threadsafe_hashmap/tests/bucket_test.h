#ifndef THREADSAFE_HASHMAP_LINKED_LIST_TEST_H
#define THREADSAFE_HASHMAP_LINKED_LIST_TEST_H

#ifdef NDEBUG
#undef NDEBUG
  #define RESTORE_NDEBUG
#endif

#include <assert.h>

#ifdef RESTORE_NDEBUG
#undef RESTORE_NDEBUG
  #define NDEBUG
#endif

#include <iostream>
#include <vector>

#include "../src/bucket.h"

using std::make_pair;

using my_concurrency::internals::Bucket;

namespace tests {

class LinkedListTest {
 public:
  void TestAll() {
    SingleAppendTest();
    ClearTest();
    MultiAppendTest();
    LookupTest();
    RemoveTest();
    IteratorTest();

    std::cout << "Linked list tests passed." << std::endl;
  }

 private:
  void SingleAppendTest() {
    Bucket<int, int> list;
    list.Insert(1, 10);
    assert(1 == list.Size());
    assert(!list.Empty());
    std::cout << "\t" << __func__ << " passed" << std::endl;
  }

  void ClearTest() {
    Bucket<int, int> list;
    list.Insert(1, 10);
    list.Clear();
    assert(0 == list.Size());
    assert(list.Empty());
    std::cout << "\t" << __func__ << " passed" << std::endl;
  }

  void MultiAppendTest() {
    Bucket<int, int> list;
    for (int i = 0; i < 2; i++) {
      list.Insert(1, 10);
      list.Insert(2, 20);
      list.Insert(3, 30);

      assert(list.Size() == 3);
      list.Clear();
      assert(0 == list.Size());
    }
    std::cout << "\t" << __func__ << " passed" << std::endl;
  }

  void LookupTest() {
    Bucket<int, int> list;
    int num_elem = 10;
    for (int i = 0; i < num_elem; i++)
      if (i % 2 == 1)
        list.Insert(i, i * 10);

    assert(num_elem / 2 == list.Size());

    for (int i = 0; i < num_elem + 1; i++)
      if (i % 2 == 1) {
        auto result = list.Lookup(i);
        assert(result.first && i * 10 == result.second);
      } else {
        assert(false == list.Lookup(i).first);
      }
    std::cout << "\t" << __func__ << " passed" << std::endl;
  }

  void RemoveTest() {
    Bucket<int, int> list;
    assert(false == list.Remove(0));

    for (int i = 0; i < 10; i++)
      if (i % 2 == 0)
        list.Insert(i, i * 10);

    assert(list.Remove(0));  // remove from the beginning
    assert(make_pair(false, 0) == list.Lookup(0));
    assert(4 == list.Size());
    assert(false == list.Remove(1));

    assert(list.Remove(4));  // remove from the middle
    assert(make_pair(false, 0) == list.Lookup(4));
    assert(false == list.Remove(4));
    assert(3 == list.Size());

    assert(list.Remove(8));  // remove from the end
    assert(make_pair(false, 0) == list.Lookup(8));
    assert(false == list.Remove(8));
    assert(2 == list.Size());

    assert(list.Lookup(2).first && list.Lookup(6).first);

    list.Insert(0, 0);
    assert(make_pair(true, 0) == list.Lookup(0));
    list.Insert(20, 200);
    assert(make_pair(true, 200) == list.Lookup(20));
    assert(4 == list.Size());
    list.Clear();
    assert(0 == list.Size() && list.Empty());
    std::cout << "\t" << __func__ << " passed" << std::endl;
  }

  void IteratorTest() {
    std::vector<int> keys{1, 2, 3, 10, 20};
    Bucket<int, int> list;
    for (int x : keys)
      list.Insert(x, x * 10);

    auto answer_iter = keys.rbegin();
    for (auto iter = list.Begin(); iter != list.End(); ++iter, ++answer_iter)
      assert((*iter).first == *answer_iter && (*iter).second == *answer_iter * 10);
    std::cout << "\t" << __func__ << " passed" << std::endl;
  }
};


} // namespace tests


#endif //THREADSAFE_HASHMAP_LINKED_LIST_TEST_H
