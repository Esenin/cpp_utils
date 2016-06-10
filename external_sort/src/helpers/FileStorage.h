#ifndef EXTERNALSORT_FILESTORAGE_H
#define EXTERNALSORT_FILESTORAGE_H

#include <vector>
#include <string>

#include "environment.h"
#include "shared_file.h"

/// @class FileStorage manages file states: opens, closes, creates new ones when necessary
class FileStorage {
 public:
  /// @param in_filepath full or relative path to file with input data
  /// @param out_filepath full or relative path to file where to store sorted data
  FileStorage(const char *in_filepath, const char *out_filepath);

  /// @brief Input file handler getter. Lazy initialization
  /// @warning Produce error exit in case if cannot open file
  raii::SharedFile InputFile();

  /// @brief Output file handler getter. Lazy initialization
  /// @warning Produce error exit in case if cannot create / open file
  raii::SharedFile OutputFile();

  /// @brief Get spicific file handler from temp files list
  /// @param index file index in list. Must be "> 0" and "< number of temp files"
  raii::SharedFile GetTempFile(int index);

  /// @brief Creates and opens new temporary (program-running lifetime) file
  /// Uses standart linux method for creating temporary files and creates file in output directory
  /// if first method failed
  /// @warning Produce error if cannot create file in both cases
  raii::SharedFile CreateNewTempFile();

 private:
  /// @brief Extracts directory from output file path
  /// @returns directory path or empty string
  std::string GetOutputDirectory() const;

  const std::string kInFilepath_;
  const std::string kOutFilepath_;

  raii::SharedFile in_file_;
  raii::SharedFile out_file_;
  std::vector<raii::SharedFile> temp_files_;
};

using SharedFileStorage = std::shared_ptr<FileStorage>;

#endif //EXTERNALSORT_FILESTORAGE_H
