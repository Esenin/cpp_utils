#ifndef RECURSIVEFUNCTION_HELPERS_H
#define RECURSIVEFUNCTION_HELPERS_H

#include <inttypes.h>

namespace helpers {

  static uint64_t BitCounter(uint64_t x) {
  x = x - ((x >> 1) & 0x5555555555555555ull);
  x = (x & 0x3333333333333333ull) + ((x >> 2) & 0x3333333333333333ull);
  return (((x + (x >> 4)) & 0xF0F0F0F0F0F0F0Full) * 0x101010101010101ull) >> 56;
}

}

#endif //RECURSIVEFUNCTION_HELPERS_H
