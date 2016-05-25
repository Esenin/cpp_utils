#include <iostream>

#include "include/threadsafe_hashmap.h"
#include "tests/linked_list_test.h"

using namespace std;
int main() {

  tests::LinkedListTest list_test;
  list_test.TestAll();

  return 0;
}