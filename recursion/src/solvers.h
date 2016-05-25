#ifndef RECURSIVEFUNCTION_RECURSIVEFUNCTION_H
#define RECURSIVEFUNCTION_RECURSIVEFUNCTION_H

#include <inttypes.h>
#include <map>

#include "helpers.h"

using helpers::BitCounter;

namespace solvers {

static uint64_t DirectCompute(uint64_t x)  {
  if (x <= 1) return 1;

  return DirectCompute(x / 2) + ((x & 1) == 1 ? DirectCompute(x / 2 - 1) : 0);
}


static uint64_t TreeBfsCompute(uint64_t n) {
  std::map<uint64_t, uint64_t> tree {{n, 1}};

  while (tree.rbegin()->first > 1) {

    auto node = *tree.rbegin();
    tree.erase(--tree.end());

    if (BitCounter(node.first) == 1) {
      tree[1] += node.second;
      continue;
    }

    auto main_prev_val = node.first / 2;
    tree[main_prev_val] += node.second;
    if ((node.first & 1) == 1)
      tree[main_prev_val - 1] += node.second;
  }

  uint64_t sum = 0;
  for (const auto &x : tree) {
    sum += x.second;
  }
  return sum;
}

} // namespace solvers



#endif //RECURSIVEFUNCTION_RECURSIVEFUNCTION_H
