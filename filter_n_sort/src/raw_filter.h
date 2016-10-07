#ifndef TEST_TASK_FIND_REPLACE_FILTER_H_H
#define TEST_TASK_FIND_REPLACE_FILTER_H_H

#include <cstdio>
#include <vector>

#include "filter_base.h"

namespace app {

/// @brief RawFilter is two-pass filter. At first pass it collects all occurrences of a pattern
///        At second pass it copies only the segments which were approved
class RawFilter : public FilterBase {
 public:
  /// @throws runtime_error if cannot open file
  RawFilter(const std::string &input_filename, const std::string &target_word);
  ~RawFilter();

  bool HasNext() const override;

  std::string NextLine() override;

 private:
  /// @brief Read next line to a memoty buffer using getc
  /// @warning may work incorrectly on WIndows due to different line-endings ('\r')
  /// @return line size
  size_t FetchLineToBuffer();

  void SkipSpaces(size_t &pos, size_t len) const;
  void SkipTilSpace(size_t &pos, size_t len) const;

  /// @brief finds pattern matches and its length with corresponding space-characters
  void FindPatternOccurrences(std::vector<std::pair<size_t, size_t>> &occurrences, size_t buffer_size) const;
  const size_t kBufInitSize = 4 * 1024;
  FILE *in_file_ = nullptr;

  std::vector<char> buffer_;
};

} // namespace app


#endif //TEST_TASK_FIND_REPLACE_FILTER_H_H
