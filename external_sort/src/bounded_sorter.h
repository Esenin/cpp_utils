#ifndef EXTERNALSORT_BOUNDEDSORTER_H
#define EXTERNALSORT_BOUNDEDSORTER_H

#include "dynamic_chunk.h"
#include "helpers/FileStorage.h"

//! @class BoundedSorter is responisble for sort phases control and limited memory distribution
class BoundedSorter {
 public:
  /// @param storage which is capable to operate with input/output and temporary files
  /// @param memory limit. This amount of memory is distributed between chunks and can be fully used by them
  /// @param file_size size of input file
  BoundedSorter(SharedFileStorage &storage, int64_t memory, int64_t file_size);

  /// @brief Starts sort of two phases: split-sort and k-merge. Stores result in output file
  void Sort();

 private:
  /// @brief loads as many data as possible, sort and store to temporary (in some case in result) file
  void SplitSort();

  /// @brief loads chunks partially to memory and merge them to output buffer first, then flushes to output file
  /// uses heap to find chunk with minimum value to use
  void KWayMerge();

  const int64_t kMemoryLimit_;
  const bool kIsDataFitsInMemory; /// if all the data fits in available memory then only first phase is used

  SharedFileStorage storage_;

  int chunks_num_ = 0;
  bool has_last_line_eol_ = true; /// tracks consistency of last newline in input and output files
};


#endif //EXTERNALSORT_BOUNDEDSORTER_H
