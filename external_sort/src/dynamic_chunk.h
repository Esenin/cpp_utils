#ifndef EXTERNALSORT_DYNAMICCHUNK_H
#define EXTERNALSORT_DYNAMICCHUNK_H

#include <deque>
#include <inttypes.h>
#include <vector>

#include "helpers/shared_file.h"
#include "helpers/uniq_line.h"

/// @class DynamicChunk provides chunk data processing techniques with bounded memory limit
class DynamicChunk {
 public:
  using UniqLines = std::deque<raii::UniqLine>;

  /// @param src_file file from which chunk loads data. Can be empty then use Append to insert data to the chunk
  /// @param memory_limit chunk is keeping it's heap and stack memory less than this value
  /// @param dest_file to this file chink flushes the data at one time. Can be set later
  DynamicChunk(const raii::SharedFile &src_file, int64_t memory_limit,
               const raii::SharedFile &dest_file = raii::ShareFile());

  /// @brief loads data from source file until end of file or chunk's memory limit is reached
  void LoadNextChunk();

  /// @brief flushes the data from chunk to destination file
  /// @param rewind_after_store if flag is set then seek to begin of file after store
  /// @param canPutEol the flag s responsible for new-line character at the end of destination file
  void StoreChunk(bool rewind_after_store, bool canPutEol);

  /// @brief sort chunk's lines by strcmp()
  void SortChunk();

  /// @brief moves line from chunk (increases free memory of the chunk)
  /// @warning be sure that the chunk is not empty
  raii::UniqLine PopLine();

  /// @brief appends line to the chunk (decrease free memory)
  /// @param uniq_line rvalue unique pointer
  void Append(raii::UniqLine &&uniq_line);

  /// @brief peek size of first line in the chunk
  /// @warning be sure that the chunk is not empty
  int64_t TopLineSize() const;

  /// @brief checks if last line in the chunk has new-line character
  bool HasLastLineEolChar() const;

  /// @brief sets destination file of the chunk (where it flushes data)
  void SetDestinationFile(const raii::SharedFile &dest_file);

  /// @brief less operator provides inverse order that is used to sort chunks in heap
  bool operator < (const DynamicChunk &rhs) const;

  /// @brief check if the chunk contains no data
  inline bool IsEmpty() const { return lines_in_mem_.empty(); }

  /// @brief check if the chunks is already reached end of input file
  /// @warning be sure that input file was provided
  bool IsInputEof() const;

  /// @brief calculates free memory considering heap and (not precisely) stack
  int64_t FreeMemoryAmount() const;

 private:
  DynamicChunk& operator= (const DynamicChunk &) = delete;
  DynamicChunk(const DynamicChunk &) = delete;

  /// @brief check if free memory is enough to load average sized line from input file
  bool CanLoadLine();

  /// @brief consider last block of lines to get average size of line
  void UpdateLineSizeStats();

  /// @brief sort lines in the chunk in parallel way using all available CPUs
  /// @warning Is not stable now. Depending on free available memory could throw exception on new thread creation
  /// @todo find consistense with available memory and additional threads requirements (std stack size is 8 MB)
  /// @param begin index of line in deque from which start sorting (inclusive)
  /// @param end right bound, exclusive
  /// @param num_cpu_available number of free CPUs
  void ParallelSort(int begin, int end, int num_cpu_available);

  const int64_t kMemoryLimit;
  raii::SharedFile src_file_;
  raii::SharedFile dest_file_;

  int64_t memory_used_by_lines_ = 0;
  int64_t average_line_size_ = 0;
  UniqLines lines_in_mem_;
};


#endif //EXTERNALSORT_DYNAMICCHUNK_H