#ifndef EXTERNALSORT_UNIQ_LINE_H
#define EXTERNALSORT_UNIQ_LINE_H

#include <cstring>
#include <inttypes.h>
#include <memory>

/// @namespace raii provides RAII behaviour structures
namespace raii {

using UniqLinePtr = std::unique_ptr<char[]>;

/// @brief UniqLine stores unique pointer to char array and it's size
/// Provides default move assignment and move copy constructor
/// provides implementation for std::less and std::greater
struct UniqLine {
  uint32_t size;
  UniqLinePtr line;

  UniqLine(uint32_t size, char char_array[])
      : size(size),
        line(char_array) { }

  bool operator<(const UniqLine &rhs) const {
    return strcmp(line.get(), rhs.line.get()) < 0;
  }

  bool operator>(const UniqLine &rhs) const {
    return strcmp(line.get(), rhs.line.get()) > 0;
  }

};

} // raii


#endif //EXTERNALSORT_UNIQ_LINE_H
