#ifndef REMOVE_N_SORT_SIMPLEFILTER_H
#define REMOVE_N_SORT_SIMPLEFILTER_H

#include <fstream>
#include <regex>

#include "filter_base.h"

namespace app {

class RegexFilter : public FilterBase {
 public:
  RegexFilter(const std::string &input_filename, const std::string &target_word);

  bool HasNext() const override;

  std::string NextLine() override;

 private:
  std::ifstream input_;
  std::regex pattern_begin_;
  std::regex pattern_end_;
  std::regex pattern_mid_;
};

}

#endif //REMOVE_N_SORT_SIMPLEFILTER_H
