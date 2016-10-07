#ifndef REMOVE_N_SORT_SIMPLEFILTER_H
#define REMOVE_N_SORT_SIMPLEFILTER_H

#include <fstream>
#include <regex>

#include "filter_base.h"

namespace app {

/// @brief RegexFilter is a pretty straight-forward filter based on regex_replace
///        In order to keep track on correspoing whitespaces the pattern is divided to match the beginning, middle
///        and end of the line separately
class RegexFilter : public FilterBase {
 public:
  /// @throws runtime_error if cannot open file
  RegexFilter(const std::string &input_filename, const std::string &target_word);

  bool HasNext() const override;

  std::string NextLine() override;

 private:
  std::ifstream input_;
  std::regex pattern_begin_;
  std::regex pattern_end_;
  std::regex pattern_mid_;
};

} // namespace app

#endif //REMOVE_N_SORT_SIMPLEFILTER_H
