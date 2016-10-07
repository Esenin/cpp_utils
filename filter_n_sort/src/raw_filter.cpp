#include "raw_filter.h"

#include <cstring>
#include <stdexcept>

using namespace app;

RawFilter::RawFilter(const std::string &input_filename, const std::string &target_word)
    : FilterBase(input_filename, target_word),
      buffer_(kBufInitSize, '\0') {
  in_file_ = fopen(input_filename.c_str(), "r");
  if (!in_file_)
    throw std::runtime_error("Cannot open file");
}

bool RawFilter::HasNext() const {
  return in_file_ && !feof(in_file_);
}

size_t RawFilter::FetchLineToBuffer() {
  size_t buffer_used = 0;

  char current_char = '\n';
  while ((current_char = getc(in_file_)) != EOF) {
    if ('\n' == current_char)
      break;
    buffer_[buffer_used++] = current_char;
    if (buffer_used + 1 == buffer_.size())
      buffer_.resize(buffer_.size() * 2);
  }
  buffer_[buffer_used] = '\0';
  return buffer_used;
}

void RawFilter::SkipSpaces(size_t &pos, size_t len) const {
  while (pos < len && isspace(buffer_[pos]))
    ++pos;
}

void RawFilter::SkipTilSpace(size_t &pos, size_t len) const {
  while (pos < len && !isspace(buffer_[pos]))
    ++pos;
}

std::string RawFilter::NextLine() {
  size_t buffer_used = FetchLineToBuffer();

  if (buffer_used < kTargetWord.size()) {
    return std::string(buffer_.data(), buffer_used);
  }

  std::vector<std::pair<size_t, size_t>> occurrences; // (start_pos, size)
  FindPatternOccurrences(occurrences, buffer_used);

  size_t skip_size = 0;
  for (const auto &match_p : occurrences) {
    skip_size += match_p.second;
  }

  std::string result;
  result.reserve(std::max((size_t)0, buffer_used - skip_size + 1));
  size_t cur_pos = 0;
  for (const auto &match_p : occurrences) {
    while (cur_pos < match_p.first) {
      result.push_back(buffer_[cur_pos++]);
    }
    cur_pos += match_p.second;
  }
  while (cur_pos < buffer_used)
    result.push_back(buffer_[cur_pos++]);

  return result;
}

void RawFilter::FindPatternOccurrences(std::vector<std::pair<size_t, size_t>> &occurrences, size_t buffer_size) const {
  const size_t kPatternSize = kTargetWord.size();
  size_t cur_pos = 0;

  // Check begin case
  bool matched = true;
  for (size_t i = 0; i < kPatternSize; i++)
    if (buffer_[i] != kTargetWord[i]) {
      matched = false;
      cur_pos = i;
      SkipTilSpace(cur_pos, buffer_size);
      break;
    }
  if (matched && (buffer_size == kPatternSize || isspace(buffer_[kPatternSize]))) {
    size_t size = kPatternSize + (buffer_size == kPatternSize ? 0 : 1);
    occurrences.push_back(std::make_pair(0, size));
    cur_pos = kPatternSize;
  } else {
    SkipTilSpace(cur_pos, buffer_size);
  }

  while (cur_pos < buffer_size) {
    SkipSpaces(cur_pos, buffer_size);

    matched = true;
    for (size_t i = 0; i < kPatternSize; i++) {
      if (cur_pos + i < buffer_size && buffer_[cur_pos + i] == kTargetWord[i]) {
        continue;
      } else {
        matched = false;
        cur_pos = cur_pos + i;
        break;
      }
    }
    if (matched && (isspace(buffer_[cur_pos + kPatternSize]) || cur_pos + kPatternSize == buffer_size)) {
      size_t size = kPatternSize + 1; // for space
      size_t start_pos = cur_pos + (cur_pos + kPatternSize == buffer_size ? -1 : 0); // rm front space if end of line
      occurrences.push_back(std::make_pair(start_pos, size));
      cur_pos += kPatternSize;
    } else {
      SkipTilSpace(cur_pos, buffer_size);
    }
  }
}

RawFilter::~RawFilter() {
  if (in_file_)
    fclose(in_file_);
}

