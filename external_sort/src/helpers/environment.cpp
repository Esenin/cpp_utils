#include "environment.h"

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>

void environment::ErrorExit(const std::string &reason) {
  std::cerr << "ERR: " << reason << std::endl;
  exit(EXIT_FAILURE);
}

bool environment::TrySetMemoryLimit(uint64_t bytes) {
  struct rlimit limits;
  getrlimit(RLIMIT_DATA, &limits);
  limits.rlim_cur = bytes;
  return 0 == setrlimit(RLIMIT_AS, &limits);
}

int64_t environment::FileSize(const char *filepath) {
  struct stat file_stats;
  const int kExitSuccess = 0;
  if (kExitSuccess == stat(filepath, &file_stats)) {
    return file_stats.st_size;
  }
  return -1;
}

void environment::Assert(bool cond, std::string msg) {
  if (!cond)
    ErrorExit(msg);
}
