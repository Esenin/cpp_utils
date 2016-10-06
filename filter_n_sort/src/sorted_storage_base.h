#ifndef REMOVE_N_SORT_SORTEDSTORAGEBASE_H
#define REMOVE_N_SORT_SORTEDSTORAGEBASE_H

#include <string>

namespace app {

class SortedStorageBase {
 public:
  SortedStorageBase(const std::string &filename) : out_filename_(filename) {}

  virtual ~SortedStorageBase() {}

  virtual void AddLine(std::string &&line) = 0;

  void Flush() {
    SortHook();
    FlushHook();
  }

 protected:
  virtual void SortHook() {}
  virtual void FlushHook() = 0;

  std::string out_filename_;
};

}

#endif //REMOVE_N_SORT_SORTEDSTORAGEBASE_H
