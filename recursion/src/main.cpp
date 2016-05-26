#ifdef NDEBUG
#undef NDEBUG
  #define RESTORE_NDEBUG // let assert works in testing purposes
#endif

#include <assert.h>

#ifdef RESTORE_NDEBUG
#undef RESTORE_NDEBUG
  #define NDEBUG
#endif


#include <chrono>
#include <functional>
#include <inttypes.h>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

#include "solvers.h"

using solvers::DirectCompute;
using solvers::TreeBfsCompute;
using std::cout;
using std::endl;

void BitCounterTest() {
  for (uint32_t i = 1; i < 100; i++) {
    uint32_t x = i;
    uint32_t counter = 0;

    while (x) {
      x = x & (x - 1);
      counter++;
    }
    assert(counter == BitCounter(i));

  }
}

/// @brief checks function for correct answers
/// @param solver is any function within solvers namespace
void PrecalculatedTests(std::function<uint64_t(uint64_t)> solver) {
  const std::vector<uint64_t> input {0, 1, 2, 3, 256, 258, 1023, 1024, 123456, 987654321, 123456789012345};
  const std::vector<uint64_t> answers {1, 1, 1, 2, 1, 14, 89, 1, 59, 167576, 140274140};

  for (int i = 0; i < input.size(); i++) {
    assert(solver(input[i]) == answers[i]);
  }
}

void ForwardTest() {
  for (uint64_t i = 0; i < 1000; i++)
    assert(TreeBfsCompute(i) == DirectCompute(i));
}

void RandomizedTest() {
  std::default_random_engine generator;
  std::uniform_int_distribution<uint32_t> input_stream;

  int num_probes = 100;
  for (int i = 0; i < num_probes; i++) {
    uint64_t input = input_stream(generator);
    assert(TreeBfsCompute(input) == DirectCompute(input));
  }

}

void TestSuit() {
  std::cout << "Testing in progress..." << std::endl;
  PrecalculatedTests(DirectCompute);
  PrecalculatedTests(TreeBfsCompute);
  std::cout << "Precalculated tests passed," << std::endl;
  ForwardTest();
  std::cout << "Forward tests passed," << std::endl;
  RandomizedTest();

  std::cout << "All tests passed" << std::endl;
}

void TimeMeasurement() {
  const int kNumRepeats = 50;
  const uint64_t kValue = 123456789012345678;

  double elapsed_usec = 0;
  for (int i = 0; i < kNumRepeats; i++) {
    auto start_time = std::chrono::steady_clock::now();

    TreeBfsCompute(kValue);

    auto stop_time = std::chrono::steady_clock::now();
    elapsed_usec += std::chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time).count();
    std::this_thread::sleep_for(std::chrono::milliseconds( 10 )); // invalidate CPU's pipeline
  }

  std::cout << "Average execution time for f(123456789012345678): " << elapsed_usec / kNumRepeats << " [us]"
      << std::endl;

}

int main() {
  BitCounterTest();
  TestSuit();
  TimeMeasurement();

  std::cout << "f(123456789012345678) = " << TreeBfsCompute(123456789012345678) << std::endl;

  return 0;
}