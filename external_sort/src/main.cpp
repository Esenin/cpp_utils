#include <cstring>

#include <iostream>

#include "bounded_sorter.h"
#include "helpers/FileStorage.h"
#include "helpers/environment.h"

using namespace std;
using namespace environment;

uint64_t ParseMemoryLimitParam(const char *mem_arg) {
  const size_t size = strlen(mem_arg);
  if (!size)
    return 0;
  size_t scale_coef = 1;
  const char modifier = mem_arg[size - 1];
  switch (modifier) {
    case 'G' :
      scale_coef *= 1024;
    case 'M' :
      scale_coef *= 1024;
    case 'K' :
      scale_coef *= 1024;
    case 'B' :
      scale_coef *= 1;
    default:
      break;
  }
  return scale_coef * strtoll(mem_arg, nullptr, 10);
}

static void Sort(const char *in_filepath, const char *out_filepath, int64_t data_size, int64_t mem_size) {
  const int64_t kMemoryReserveMin = 300 * 1024 * 1024; // 300 MB
  const int64_t kMemoryReserveMax = kMemoryReserveMin + 1024 * 1024 * 1024; // 1 GB for OS
  int64_t mem_to_reserve_ratio = mem_size / kMemoryReserveMax;
  int64_t memory_for_heap = 0;
  if (mem_to_reserve_ratio >= 2) {
    memory_for_heap = mem_size - kMemoryReserveMax;
  } else {
    const int64_t kStackMemory = mem_size / 2; // for secondary objects
    memory_for_heap = max(mem_size - kMemoryReserveMin, mem_size - kStackMemory);
  }

  Assert(memory_for_heap > 0, "The program requires more memory");

  auto storage = std::make_shared<FileStorage>(in_filepath, out_filepath);

  BoundedSorter sorter(storage, memory_for_heap, data_size);
  sorter.Sort();
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    cerr << "Usage: external_sort <input file> <output-file> <memory limit>[G|M|K|B(default)]\n"
        << "Example with 1 Gb: ./external_sort input.txt output.txt 1G" << endl;
    exit(EXIT_FAILURE);
  }

  const char *input_filepath = argv[1];
  const char *output_filepath = argv[2];
  uint64_t memory_limit = ParseMemoryLimitParam(argv[3]);

  if (0 == memory_limit) {
    cerr << "Error: Check memory limits" << endl;
    exit(EXIT_FAILURE);
  }

  if (!TrySetMemoryLimit(memory_limit)) {
    cerr << "Cannot set memory limit. Is your system POSIX capable? Or try to check memory param\n"
        << "errno: " << errno << endl;
    exit(EXIT_FAILURE);
  }

  cout << "Sorting..." << endl;
  Sort(input_filepath, output_filepath, FileSize(input_filepath), memory_limit);

  cout << "Done" << endl;
  return EXIT_SUCCESS;
}