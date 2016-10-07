#ifndef TEST_TASK_FIND_REPLACE_FILTER_H_H
#define TEST_TASK_FIND_REPLACE_FILTER_H_H

#include <cstdio>
#include <vector>

#include "filter_base.h"

namespace app {

class RawFilter : public FilterBase {
 public:
  RawFilter(const std::string &input_filename, const std::string &target_word);
  ~RawFilter();

  bool HasNext() const override;

  std::string NextLine() override;

 private:
  size_t FetchLineToBuffer();
  void SkipSpaces(size_t &pos, size_t len) const;
  void FindPatternOccurrences(std::vector<std::pair<size_t, size_t>> &occurrences, size_t buffer_used) const;

  void SkipTilSpace(size_t &pos, size_t len) const;
  const size_t kBufInitSize = 4 * 1024;
  FILE *in_file_ = nullptr;

  std::vector<char> buffer_;
};

}


#endif //TEST_TASK_FIND_REPLACE_FILTER_H_H
