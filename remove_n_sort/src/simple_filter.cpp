#include "simple_filter.h"

#include <assert.h>

using namespace app;

SimpleFilter::SimpleFilter(const std::string &input_filename, const std::string &target_word)
    : FilterBase(input_filename, target_word),
    input_(input_filename),
    pattern_(target_word) {
}

bool SimpleFilter::HasNext() const {
  return !input_.eof();
}

std::string SimpleFilter::NextLine() {
  std::string line;
  std::getline(input_, line);

  line = std::regex_replace(line, pattern_, "");
  return line;
}
