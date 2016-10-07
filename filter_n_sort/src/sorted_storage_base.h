#ifndef REMOVE_N_SORT_SORTEDSTORAGEBASE_H
#define REMOVE_N_SORT_SORTEDSTORAGEBASE_H

#include <string>

namespace app {

/// @brief SortedStorageBase accumulates lines and saves them sorted
class SortedStorageBase {
 public:
  /// @param output filepath
  SortedStorageBase(const std::string &filename) : out_filename_(filename) {}

  virtual ~SortedStorageBase() {}

  /// @brief in-place add line to the storage
  virtual void AddLine(std::string &&line) = 0;

  /// @brief template method to make sure inherited class won't forget to sort content
  void Flush() {
    SortHook();
    FlushHook();
  }

 protected:
  virtual void SortHook() {}
  virtual void FlushHook() = 0;

  std::string out_filename_;
};

} // namespace app

#endif //REMOVE_N_SORT_SORTEDSTORAGEBASE_H
