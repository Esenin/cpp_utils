#ifndef THREADSAFE_HASHMAP_LINKED_LIST_TEST_H
#define THREADSAFE_HASHMAP_LINKED_LIST_TEST_H

#include <assert.h>
#include <iostream>
#include <vector>

#include "../src/linked_list.h"

using my_concurrency::internals::LinkedList;

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
    LinkedList<int> list;
    assert(list.TryAppend(1));
    assert(1 == list.Size());
    assert(!list.Empty());
  }

  void ClearTest() {
    LinkedList<int> list;
    assert(list.TryAppend(1));
    list.Clear();
    assert(0 == list.Size());
    assert(list.Empty());
  }

  void MultiAppendTest() {
    LinkedList<int> list;
    for (int i = 0; i < 2; i++) {
      assert(list.TryAppend(1));
      assert(list.TryAppend(2));
      assert(list.TryAppend(3));

      assert(list.Size() == 3);
      assert(false == list.TryAppend(1));
      assert(false == list.TryAppend(2));
      assert(false == list.TryAppend(3));

      list.Clear();
      assert(0 == list.Size());
    }
  }

  void LookupTest() {
    LinkedList<int> list;
    int num_elem = 10;
    for (int i = 0; i < num_elem; i++)
      if (i % 2 == 1)
        list.TryAppend(i);

    assert(num_elem / 2 == list.Size());

    for (int i = -1; i < num_elem + 1; i++)
      assert((i % 2 == 1) == list.Lookup(i));
  }

  void RemoveTest() {
    LinkedList<int> list;

    for (int i = 0; i < 10; i++)
      if (i % 2 == 0)
        list.TryAppend(i);

    assert(list.TryRemove(0));  // remove from the beginning
    assert(false == list.Lookup(0));
    assert(4 == list.Size());
    assert(false == list.TryRemove(1));

    assert(list.TryRemove(4));  // remove from the middle
    assert(false == list.Lookup(4));
    assert(false == list.TryRemove(4));
    assert(3 == list.Size());

    assert(list.TryRemove(8));  // remove from the end
    assert(false == list.Lookup(8));
    assert(false == list.TryRemove(8));
    assert(2 == list.Size());

    assert(list.Lookup(2) && list.Lookup(6));

    assert(list.TryAppend(0));
    assert(list.Lookup(0));
    assert(list.TryAppend(20));
    assert(list.Lookup(20));
    assert(4 == list.Size());
    list.Clear();
    assert(0 == list.Size() && list.Empty());
  }

  void IteratorTest() {
    std::vector<int> values {1, 2, 3, 10, 20};
    LinkedList<int> list;
    for (int x : values)
      list.TryAppend(x);

    int idx = 0;

    for (auto iter = list.begin(); iter != list.end(); ++iter, idx++)
      assert(idx < values.size() && *iter == values[idx]);

  }
};


#ifdef RESTORE_NDEBUG
  #undef RESTORE_NDEBUG
  #define NDEBUG
#endif

} // namespace tests


#endif //THREADSAFE_HASHMAP_LINKED_LIST_TEST_H
