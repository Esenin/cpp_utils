#ifndef RECURSIVEFUNCTION_RECURSIVEFUNCTION_H
#define RECURSIVEFUNCTION_RECURSIVEFUNCTION_H

#include <inttypes.h>
#include <map>

#include "helpers.h"

using helpers::BitCounter;

/// @brief solvers contains functions for computation following recursive problem:
/// f(0) = f(1) = 1, f(2n) = f(n), f(2n + 1) = f(n) + f(n - 1)
namespace solvers {

/// @brief computes recursive problem by direct evaluation the recursive statement
/// @param n  argument to compute
/// @return f(n) as defined for the namespace
static uint64_t DirectCompute(uint64_t n)  {
  if (n <= 1) return 1;

  return DirectCompute(n / 2) + ((n & 1) == 1 ? DirectCompute(n / 2 - 1) : 0);
}


/// @brief computes recursive problem using breadth-first-search approach for dependency tree
/// @param n  argument to compute
/// @return f(n) as defined for the namespace
static uint64_t TreeBfsCompute(uint64_t n) {
  std::map<uint64_t, uint64_t> tree {{n, 1}};

  while (tree.rbegin()->first > 1) {

    auto node = *tree.rbegin();
    tree.erase(--tree.end());

    // Notice:  f(2^i) = 1 for any positive i
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
