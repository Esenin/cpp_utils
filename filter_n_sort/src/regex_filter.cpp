#include "regex_filter.h"

using namespace app;

RegexFilter::RegexFilter(const std::string &input_filename, const std::string &target_word)
    : FilterBase(input_filename, target_word),
      input_(input_filename),
      pattern_begin_("^" + target_word + "([ ]|$)", std::regex_constants::optimize),
      pattern_end_("[ ]" + target_word + "$", std::regex_constants::optimize),
      pattern_mid_("[ ]" + target_word + "[ ]", std::regex_constants::optimize) {
  if (!input_.is_open())
    throw std::runtime_error("Cannot open file");
}


bool RegexFilter::HasNext() const {
  return input_.is_open() && !input_.eof();
}

std::string RegexFilter::NextLine() {
  std::string line;
  std::getline(input_, line);

  size_t line_size = 0;
  do {
    line_size = line.size();
    line = std::regex_replace(line, pattern_mid_, " ");
  } while (line.size() < line_size);
  
  line = std::regex_replace(line, pattern_end_, "");
  line = std::regex_replace(line, pattern_begin_, "");
  return line;
}
