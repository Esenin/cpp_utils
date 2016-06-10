#include "dynamic_chunk.h"

#include <algorithm>
#include <thread>
#include "helpers/environment.h"

using namespace raii;
using namespace environment;

DynamicChunk::DynamicChunk(const SharedFile &src_file, int64_t memory_limit, const SharedFile &dest_file)
    : kMemoryLimit(memory_limit),
      src_file_(src_file),
      dest_file_(dest_file) { }

void DynamicChunk::LoadNextChunk() {
  const int kStatsUpdateFreq = 100 * 1000;
  const int kEofFlag = -1;
  int statistics_counter = 0;
  char *line = nullptr;
  size_t dummy = 0;
  ssize_t bytes_read = 0;
  try {
    while (CanLoadLine()
        && kEofFlag != (bytes_read = sizeof(char) * getline(&line, &dummy, src_file_->file))) {
      Assert(nullptr != line, "*line == NULL");
      lines_in_mem_.push_back(UniqLine {static_cast<uint32_t>(bytes_read), line});
      line = nullptr;

      memory_used_by_lines_ += bytes_read + 1 /* for zero-terminated char */;
      statistics_counter++;
      if (statistics_counter > kStatsUpdateFreq) {
        statistics_counter = 0;
        UpdateLineSizeStats();
      }
    }
  } catch (std::bad_alloc &e) {
    // nothing to do. Next chunk handles rest of the input file
    DEBUG("bad_alloc on loading chunk:\t" << e.what());
    DEBUG("\tfree memory available for this chunk : " << FreeMemoryAmount() / 1024 << " [KB]");
  }

  UpdateLineSizeStats();
}

bool DynamicChunk::CanLoadLine() {
  const int64_t kAvgPageSize = 4 * 1024; // 4 KB
  const int kMinStatInfo = 15;
  if (0 == average_line_size_ && lines_in_mem_.size() > kMinStatInfo)
    UpdateLineSizeStats();

  return average_line_size_ + kAvgPageSize < FreeMemoryAmount();
}

void DynamicChunk::StoreChunk(bool rewind_after_store, bool canPutEol) {
  Assert(nullptr != dest_file_, "Cannot store chunk. Set destination file first");
  const char *kWritingErrorMsg = "Writing error occurred";
  const int kWriteSuccess = 0;
  const char kEOL = '\n';

  for (int i = 0; i < lines_in_mem_.size(); i++) {
    const auto &string = lines_in_mem_[i];
    char &last_char = string.line.get()[string.size - 1];
    const bool kHasNewLine = kEOL == last_char;
    const bool kIsLastLine = i == lines_in_mem_.size() - 1;

    if (kIsLastLine && kHasNewLine && !canPutEol)
      last_char = '\0';

    Assert(fputs(string.line.get(), dest_file_->file) >= kWriteSuccess, kWritingErrorMsg);

    if (!kHasNewLine && (!kIsLastLine || (kIsLastLine && canPutEol))) {
      Assert(fputc(kEOL, dest_file_->file) >= kWriteSuccess, kWritingErrorMsg);
    }
  }
  if (rewind_after_store)
    rewind(dest_file_->file);
  lines_in_mem_.clear();
  memory_used_by_lines_ = 0;
}

void DynamicChunk::UpdateLineSizeStats() {
  average_line_size_ = 0;
  const uint32_t  kStatInfoEnough = 15;
  const uint32_t kCount = std::min(static_cast<uint32_t>(lines_in_mem_.size()), kStatInfoEnough);
  for (int i = 0; i < kCount; i++)
    average_line_size_ += lines_in_mem_[lines_in_mem_.size() - i - 1].size;
  average_line_size_  = (kCount)? average_line_size_ / kCount : 0;
}

bool DynamicChunk::operator<(const DynamicChunk &rhs) const {
  return lines_in_mem_.front() > rhs.lines_in_mem_.front();
}

void DynamicChunk::SortChunk() {
  std::sort(lines_in_mem_.begin(), lines_in_mem_.end());
  // ParallelSort(0, lines_in_mem_.size(), std::thread::hardware_concurrency());
}

bool DynamicChunk::IsInputEof() const {
  return feof(src_file_->file) != 0;
}

UniqLine DynamicChunk::PopLine() {
  UniqLine top_line(std::move(lines_in_mem_.front()));
  lines_in_mem_.pop_front();
  memory_used_by_lines_ -= top_line.size;
  return top_line;
}

void DynamicChunk::Append(UniqLine &&uniq_line) {
  lines_in_mem_.push_back(std::forward<UniqLine>(uniq_line));
  memory_used_by_lines_ += uniq_line.size;
}

void DynamicChunk::SetDestinationFile(const SharedFile &dest_file) {
  dest_file_ = dest_file;
}

int64_t DynamicChunk::TopLineSize() const {
  return lines_in_mem_.front().size;
}

bool DynamicChunk::HasLastLineEolChar() const {
  if (lines_in_mem_.empty())
    return false;
  const UniqLine &last_line = lines_in_mem_.back();
  return '\n' == last_line.line.get()[last_line.size - 1];
}

int64_t DynamicChunk::FreeMemoryAmount() const {
  const int64_t kAvgPageSize = 4 * 1024 * 1024;
  const int64_t kLineSize = sizeof(decltype(lines_in_mem_)::value_type);
  const int64_t kElementsPerPage = kAvgPageSize / kLineSize;
  const int64_t kDequeMappingSize = lines_in_mem_.size() / kElementsPerPage * sizeof(uint64_t) * 2;
  int64_t memory_used = memory_used_by_lines_ +
                        kDequeMappingSize +
                        lines_in_mem_.size() * sizeof(decltype(lines_in_mem_)::value_type);
  return kMemoryLimit - memory_used;
}

void DynamicChunk::ParallelSort(int begin, int end, int num_cpu_available) {
  if (end - 1 - begin <= 0)
    return;
  int left = begin;
  int right = end - 1;
  /*
   * We have to store pivot line because const ref to uniq element will be broken after swap
   * */
  const char *kPivotLine = lines_in_mem_[(left + right) / 2].line.get();

  while (left <= right)
  {
    while (strcmp(lines_in_mem_[left].line.get(), kPivotLine) < 0)
      left++;
    while (strcmp(lines_in_mem_[right].line.get(), kPivotLine) > 0)
      right--;
    if (left <= right)
    {
      std::swap(lines_in_mem_[left], lines_in_mem_[right]);
      right--;
      left++;
    }
  }

  std::thread left_part_thread;
  const bool kHasRightPart = begin < right;
  if (left < end - 1) {
    if (num_cpu_available > 1) {
      int half_free_cpu = kHasRightPart? num_cpu_available - num_cpu_available / 2 : num_cpu_available;
      num_cpu_available -= half_free_cpu;
      left_part_thread = std::thread(&DynamicChunk::ParallelSort, this, left, end, half_free_cpu);
    }
    else
      std::sort(lines_in_mem_.begin() + left, lines_in_mem_.begin() + end);
  }

  if (kHasRightPart) {
    if (num_cpu_available > 1)
      ParallelSort(begin, right + 1, num_cpu_available);
    else
      std::sort(lines_in_mem_.begin() + begin, lines_in_mem_.begin() + right + 1);
  }

  if (left_part_thread.joinable()) {
    left_part_thread.join();
  }
}
