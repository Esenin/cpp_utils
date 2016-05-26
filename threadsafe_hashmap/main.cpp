#include <iostream>

#include "tests/linked_list_test.h"
#include "tests/concurrent_list_test.h"
#include "tests/concurrent_map_test.h"

using namespace std;
int main() {

  tests::LinkedListTest list_test;
  list_test.TestAll();

  tests::ConcurrentListTest concurrent_test;
  concurrent_test.TestAll();

  tests::ConcurrentMapTest map_test;
  map_test.TestAll();

  return 0;
}