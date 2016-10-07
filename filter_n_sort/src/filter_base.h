#ifndef REMOVE_N_SORT_FILTERBASE_H
#define REMOVE_N_SORT_FILTERBASE_H

#include <string>

namespace app {

/// @brief Filter object performs reading of input file and removing a given word from it
class FilterBase {
 public:
  /// @param input_filename input filepath
  /// @param target_word word to be excluded
  FilterBase(const std::string &input_filename, const std::string &target_word)
      : in_filename_(input_filename),
        kTargetWord(target_word) {}

  virtual ~FilterBase() {}

  /// @brief checks if one more line is available
  virtual bool HasNext() const = 0;

  /// @brief Get next line. Make sure if another line is available first
  virtual std::string NextLine() = 0;

 protected:
  std::string in_filename_;
  const std::string kTargetWord;
};

} // namespace app

#endif //REMOVE_N_SORT_FILTERBASE_H
