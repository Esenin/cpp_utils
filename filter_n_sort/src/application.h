#ifndef REMOVE_N_SORT_APPLICATION_H
#define REMOVE_N_SORT_APPLICATION_H

#include <memory>

#include "filter_base.h"
#include "sorted_storage_base.h"

namespace app {

using UniqFilter = std::unique_ptr<FilterBase>;
using UniqStorage = std::unique_ptr<SortedStorageBase>;

/// @brief Application filters input file by a given word and then sorts the result case-insensitively
class Application {
 public:
  Application(UniqFilter &&filter, UniqStorage &&storage)
      : filter_(std::move(filter)), storage_(std::move(storage)) {}


  void Run() {
    while (filter_->HasNext()) {
      storage_->AddLine(filter_->NextLine());
    }

    storage_->Flush();
  }

 private:
  UniqFilter filter_;
  UniqStorage storage_;
};

} // namespace app

#endif //REMOVE_N_SORT_APPLICATION_H
