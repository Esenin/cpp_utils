#include <iostream>

#include "tests/bucket_test.h"
#include "tests/concurrent_bucket_test.h"
#include "tests/hashmap_test.h"

using namespace std;
int main() {

  tests::LinkedListTest list_test;
  list_test.TestAll();

  tests::ConcurrentListTest concurrent_test;
  concurrent_test.TestAll();

  tests::ConcurrentMapTest map_test;
  map_test.TestAll();

  std::cout << "All tests passed.\n" << std::endl;
  return 0;
}