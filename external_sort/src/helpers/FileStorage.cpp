#include "FileStorage.h"

using namespace environment;
using namespace raii;

FileStorage::FileStorage(const char *in_filepath, const char *out_filepath)
    : kInFilepath_(in_filepath),
      kOutFilepath_(out_filepath) { }


raii::SharedFile FileStorage::InputFile() {
  if (nullptr == in_file_) {
    in_file_ = raii::ShareFile(fopen(kInFilepath_.c_str(), "rb"));
    if (nullptr == in_file_) {
      ERROR("Cannot create input file");
    }
  }
  return in_file_;
}

std::string FileStorage::GetOutputDirectory() const {
  uint64_t last_slash = kOutFilepath_.find_last_of("/");
  return kOutFilepath_.substr(0, last_slash);
}

raii::SharedFile FileStorage::OutputFile() {
  if (nullptr == out_file_) {
    out_file_ = ShareFile(fopen(kOutFilepath_.c_str(), "wb+"));
    Assert(nullptr != out_file_, "Cannot create output file. Please check permissions");
  }
  return out_file_;
}

raii::SharedFile FileStorage::GetTempFile(int index) {
  return temp_files_[index];
}

raii::SharedFile FileStorage::CreateNewTempFile() {
  auto temp_file = ShareFile(tmpfile());
  std::string filepath; // leave empty if linux::tmpfile works
  if (nullptr == temp_file->file) {
    // Sometimes error is possible. I had a few cases
    DEBUG("Cannot create temporary file using default linux tmpfile(). Trying create file in output directory");
    filepath = GetOutputDirectory();
    if (filepath.size())
      filepath += "/";
    filepath += "external_sort_tmp" + std::to_string(temp_files_.size());

    if (nullptr == (temp_file->file = fopen(filepath.c_str(), "wb+"))) {
      ERROR("Cannot create temporary file " << filepath);
    }
    temp_file->filepath = filepath;
  }

  temp_files_.push_back(temp_file);
  return temp_file;
}
