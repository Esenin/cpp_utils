#ifndef REMOVE_N_SORT_SIMPLEFILTER_H
#define REMOVE_N_SORT_SIMPLEFILTER_H

#include <fstream>
#include <regex>

#include "filter_base.h"

namespace app {

class SimpleFilter : public FilterBase {
 public:
  SimpleFilter(const std::string &input_filename, const std::string &target_word);

  bool HasNext() const override;

  std::string NextLine() override;

 private:
  std::ifstream input_;
  std::regex pattern_;
};

}

#endif //REMOVE_N_SORT_SIMPLEFILTER_H
