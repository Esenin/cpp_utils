#include "shared_file.h"


raii::SharedFile raii::ShareFile(FILE *ptr, const std::string &filepath) {
  return ::raii::SharedFile(new FileWrapper{ptr, filepath});
}