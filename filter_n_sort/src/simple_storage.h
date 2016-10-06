#ifndef REMOVE_N_SORT_SIMPLESTORAGE_H
#define REMOVE_N_SORT_SIMPLESTORAGE_H

#include <vector>

#include "sorted_storage_base.h"

namespace app {

class SimpleStorage : public SortedStorageBase {
 public:
  SimpleStorage(const std::string &filename);

  void AddLine(std::string &&line) override;

 private:
  void SortHook() override;
  void FlushHook() override;

  std::vector<std::string> lines_;
};

}

#endif //REMOVE_N_SORT_SIMPLESTORAGE_H
