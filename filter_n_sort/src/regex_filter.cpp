#include "regex_filter.h"

using namespace app;

RegexFilter::RegexFilter(const std::string &input_filename, const std::string &target_word)
    : FilterBase(input_filename, target_word),
      input_(input_filename),
      pattern_begin_("^" + target_word + "([ ]|$)", std::regex_constants::optimize),
      pattern_end_("[ ]" + target_word + "$", std::regex_constants::optimize),
      /*
       * Middle regex replacement could perform a little faster if use manual program-loop instead of
       *  regex (repeating)* mode
       */
      pattern_mid_("[ ](" + target_word + "[ ])*", std::regex_constants::optimize) {
  if (!input_.is_open())
    throw std::runtime_error("Cannot open file");
}

bool RegexFilter::HasNext() const {
  return input_.is_open() && !input_.eof();
}

std::string RegexFilter::NextLine() {
  std::string line;
  std::getline(input_, line);

  line = std::regex_replace(line, pattern_mid_, " ");
  line = std::regex_replace(line, pattern_end_, "");
  line = std::regex_replace(line, pattern_begin_, "");
  return line;
}
