#include "bounded_sorter.h"

#include <algorithm>
#include <queue>

#include "helpers/environment.h"

using namespace raii;
using namespace environment;


BoundedSorter::BoundedSorter(SharedFileStorage &storage, int64_t memory, int64_t file_size)
    : storage_(storage),
      kMemoryLimit_(memory),
      kIsDataFitsInMemory(file_size < memory) { }

void BoundedSorter::Sort() {
  DEBUG("Sort start here");
  SplitSort();
  DEBUG("Split-sorting was finished");
  if (!kIsDataFitsInMemory) {
    DEBUG("K-way merge is running. Num of chunks = " << chunks_num_);
    KWayMerge();
  }
  DEBUG("Sort finished");
}


void BoundedSorter::SplitSort() {
  const auto input_file = storage_->InputFile();
  DynamicChunk buffer(input_file, kMemoryLimit_);

  while (!feof(input_file->file)) {

    auto output_file = kIsDataFitsInMemory ? storage_->OutputFile() : storage_->CreateNewTempFile();
    buffer.LoadNextChunk();
    buffer.SetDestinationFile(output_file);

    const bool kMustSeekToBegin = true;
    const bool kDoPutEol = buffer.HasLastLineEolChar();

    has_last_line_eol_ = has_last_line_eol_ && kDoPutEol;

    buffer.SortChunk();
    buffer.StoreChunk(kMustSeekToBegin, kDoPutEol);

    chunks_num_++;
  }
}


void BoundedSorter::KWayMerge() {
  const int kOutputBufSizeScale = 2;
  const int64_t KSizePerChunk = kMemoryLimit_ / (chunks_num_ + kOutputBufSizeScale);

  DynamicChunk output_buffer(ShareFile(), kOutputBufSizeScale * KSizePerChunk, storage_->OutputFile());

  using UniqDynamicChuck = std::unique_ptr<DynamicChunk>;
  std::vector<UniqDynamicChuck> chunks; // store pointers to chunks for fast swap
  chunks.reserve(chunks_num_);

  for (int i = 0; i < chunks_num_; i++) {
    chunks.push_back(UniqDynamicChuck(new DynamicChunk(storage_->GetTempFile(i), KSizePerChunk, ShareFile())));
    chunks.back()->LoadNextChunk();
  }

  auto compare_chunks = [](const UniqDynamicChuck &lhs, const UniqDynamicChuck &rhs) {
    return *lhs < *rhs;
  };

  std::make_heap(chunks.begin(), chunks.end(), compare_chunks);
  const bool kSeekBegin = false;
  const bool kDoWriteNewLineBetweenChunks = true;

  while (!chunks.empty()) {
    std::pop_heap(chunks.begin(), chunks.end(), compare_chunks);
    auto &uniq_smallest_chunk = chunks.back();

    if (output_buffer.FreeMemoryAmount() < uniq_smallest_chunk->TopLineSize()) {
      output_buffer.StoreChunk(kSeekBegin, kDoWriteNewLineBetweenChunks);
    }
    output_buffer.Append(uniq_smallest_chunk->PopLine());

    if (uniq_smallest_chunk->IsEmpty()) {
      if (uniq_smallest_chunk->IsInputEof()) {
        chunks.pop_back();
      } else {
        uniq_smallest_chunk->LoadNextChunk();
        std::push_heap(chunks.begin(), chunks.end(), compare_chunks);
      }
    } else {
      std::push_heap(chunks.begin(), chunks.end(), compare_chunks);
    }
  }

  if (!output_buffer.IsEmpty())
    output_buffer.StoreChunk(kSeekBegin, has_last_line_eol_);
}
