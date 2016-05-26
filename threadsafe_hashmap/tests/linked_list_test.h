#ifndef THREADSAFE_HASHMAP_LINKED_LIST_TEST_H
#define THREADSAFE_HASHMAP_LINKED_LIST_TEST_H

#include <assert.h>
#include <iostream>
#include <vector>

#include "../src/concurrent_linked_list.h"

using std::make_pair;

using my_concurrency::internals::ConcurrentLinkedList;

namespace tests {

#ifdef NDEBUG
  #undef NDEBUG
  #define RESTORE_NDEBUG
#endif


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
    ConcurrentLinkedList<int, int> list;
    list.Insert(1, 10);
    assert(1 == list.Size());
    assert(!list.Empty());
  }

  void ClearTest() {
    ConcurrentLinkedList<int, int> list;
    list.Insert(1, 10);
    list.Clear();
    assert(0 == list.Size());
    assert(list.Empty());
  }

  void MultiAppendTest() {
    ConcurrentLinkedList<int, int> list;
    for (int i = 0; i < 2; i++) {
      list.Insert(1, 10);
      list.Insert(2, 20);
      list.Insert(3, 30);

      assert(list.Size() == 3);
      list.Clear();
      assert(0 == list.Size());
    }
  }

  void LookupTest() {
    ConcurrentLinkedList<int, int> list;
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
  }

  void RemoveTest() {
    ConcurrentLinkedList<int, int> list;

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
  }

  void IteratorTest() {
    std::vector<int> keys{1, 2, 3, 10, 20};
    ConcurrentLinkedList<int, int> list;
    for (int x : keys)
      list.Insert(x, x * 10);

    int idx = 0;

    for (auto iter = list.begin(); iter != list.end(); ++iter, idx++)
      assert(idx < keys.size() && (*iter).first == keys[idx] && (*iter).second == keys[idx] * 10);
  }
};


#ifdef RESTORE_NDEBUG
  #undef RESTORE_NDEBUG
  #define NDEBUG
#endif

} // namespace tests


#endif //THREADSAFE_HASHMAP_LINKED_LIST_TEST_H
