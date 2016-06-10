#ifndef EXTERNALSORT_SHARED_FILE_H
#define EXTERNALSORT_SHARED_FILE_H

#include <memory>
#include <stdio.h>

/// @namespace raii provides RAII behaviour structures
namespace raii {

/// @brief FileWrapper provides RAII management for C-File pointer
struct FileWrapper {
  FILE *file;
  std::string filepath;

  /// param _file is C FILE pointer, can be null
  /// param _filepath must be empty if file was created by linux tmpfile function because OS will deal with it
  ///     otherwise it must contain relative or full path to file. It will be closed and deleted in destructor
  FileWrapper(FILE *_file, const std::string &_filepath)
      : file(_file),
        filepath(_filepath) { }

  ~FileWrapper() {
    if (file) {
      fclose(file);
      if (filepath.size())
        remove(filepath.c_str());
    }
  }
};

using SharedFile = std::shared_ptr<FileWrapper>;
SharedFile ShareFile(FILE *ptr = nullptr, const std::string &filepath = "");

} // raii

#endif //EXTERNALSORT_SHARED_FILE_H
