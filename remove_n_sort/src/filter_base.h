#ifndef REMOVE_N_SORT_FILTERBASE_H
#define REMOVE_N_SORT_FILTERBASE_H

#include <string>

namespace app {

class FilterBase {
 public:
  FilterBase(const std::string &input_filename, const std::string &target_word)
      : in_file_(input_filename),
        target_word_(target_word) {}

  virtual ~FilterBase() {}

  virtual bool HasNext() const = 0;
  virtual std::string NextLine() = 0;

 protected:
  std::string in_file_;
  std::string target_word_;

};



}

#endif //REMOVE_N_SORT_FILTERBASE_H
