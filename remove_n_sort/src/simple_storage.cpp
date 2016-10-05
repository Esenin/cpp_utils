#include <algorithm>
#include <fstream>

#include "simple_storage.h"

app::SimpleStorage::SimpleStorage(const std::string &filename) : SortedStorageBase(filename) {}


void app::SimpleStorage::AddLine(std::string &&line) {
  lines_.push_back(std::forward<std::string>(line));
}

void app::SimpleStorage::SortHook() {
  auto str_less_insensitive =  [] (const std::string &lhs, const std::string &rhs) {
    auto min_size = std::min(lhs.size(), rhs.size());
    for (size_t i = 0; i < min_size; i++) {
      const auto kLChar = std::tolower(lhs[i]);
      const auto kRChar = std::tolower(rhs[i]);
      if (kLChar != kRChar)
        return kLChar < kRChar;
    }
    return lhs.size() < rhs.size();
  };

  const bool kHasLastLineEOL = lines_.size() && (lines_.back() == "");
  std::sort(lines_.begin(), lines_.end() + (kHasLastLineEOL? - 1 : 0) , str_less_insensitive);
}

void app::SimpleStorage::FlushHook() {
  std::ofstream out_file(out_filename_);
  if (lines_.size())
    out_file << lines_.front();
  for (size_t i = 1; i < lines_.size(); i++)
    out_file << "\n" << lines_[i];
  out_file.flush();
  out_file.close();
}

