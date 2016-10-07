#ifndef REMOVE_N_SORT_FILTERBASE_H
#define REMOVE_N_SORT_FILTERBASE_H

#include <string>

namespace app {

class FilterBase {
 public:
  FilterBase(const std::string &input_filename, const std::string &target_word)
      : in_filename_(input_filename),
        kTargetWord(target_word) {}

  virtual ~FilterBase() {}

  virtual bool HasNext() const = 0;
  virtual std::string NextLine() = 0;

 protected:
  std::string in_filename_;
  const std::string kTargetWord;

};



}

#endif //REMOVE_N_SORT_FILTERBASE_H
