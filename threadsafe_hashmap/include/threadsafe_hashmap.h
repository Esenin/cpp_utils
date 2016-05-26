#ifndef THREADSAFE_HASHMAP_THREADSAFEHASHMAP_H
#define THREADSAFE_HASHMAP_THREADSAFEHASHMAP_H

#include <assert.h>
#include <atomic>
#include <functional> // hash
#include <math.h> // sqrt
#include <memory>
#include <shared_mutex>
#include <utility> //pair

#include "../src/bucket.h"

namespace my_concurrency {

template <typename KeyType, typename ValueType>
class ThreadsafeHashmap {
 public:
  ThreadsafeHashmap(uint64_t num_buckets = 64);

  void Insert(const KeyType &key, const ValueType &value);
  std::pair<bool, ValueType> Lookup(const KeyType &key) const;
  bool Remove(const KeyType &key);

  uint64_t Size() const;
  void Clear();
  bool Empty() const;

  constexpr static bool kOperationSuccess = true;
  constexpr static bool kOperationFailed = false;
 private:
  enum class State {
    kNormal,
    kResizing
  };

  static constexpr double kIncreaseRate = 2.0;
  static constexpr double kMaxLoadFactor = 0.75;
  typedef internals::Bucket <KeyType, ValueType> Bucket;

  double LoadFactor() const;

  uint64_t Hash(const KeyType &key) const;
  uint64_t PrimaryIndex(const KeyType &key) const;
  uint64_t SecondaryIndex(const KeyType &key) const;

  void ResizingBegin();
  void ResizingDone();
  void ContiniousMoving();


  uint64_t num_buckets_primary_;
  uint64_t num_buckets_secondary_;
  std::unique_ptr<Bucket[]> primary_table_;
  std::unique_ptr<Bucket[]> secondary_table_;
  std::atomic_ullong primary_size_;
  std::atomic_ullong secondary_size_;
  std::hash<KeyType> hash_;

  State state_;
  uint64_t max_elements_to_move_; // amount of element to move at one step of incremental resizing
  mutable std::shared_timed_mutex stateupdate_mutex_;

};

template <typename KeyType, typename ValueType>
ThreadsafeHashmap<KeyType, ValueType>::ThreadsafeHashmap(uint64_t num_buckets)
    : num_buckets_primary_(num_buckets), num_buckets_secondary_(0),
      primary_table_(new Bucket[num_buckets]),
      secondary_table_(nullptr),
      primary_size_(0), secondary_size_(0),
      state_(State::kNormal)
      {}

template <typename KeyType, typename ValueType>
double ThreadsafeHashmap<KeyType, ValueType>::LoadFactor() const {
  return primary_size_.load(std::memory_order_acquire) / double(num_buckets_primary_);
}

template <typename KeyType, typename ValueType>
uint64_t ThreadsafeHashmap<KeyType, ValueType>::Hash(const KeyType &key) const {
  uint64_t hash = hash_(key);
  return hash ^ (hash >> 32);
}

template <typename KeyType, typename ValueType>
uint64_t ThreadsafeHashmap<KeyType, ValueType>::PrimaryIndex(const KeyType &key) const {
  return Hash(key) % num_buckets_primary_;
}

template <typename KeyType, typename ValueType>
uint64_t ThreadsafeHashmap<KeyType, ValueType>::SecondaryIndex(const KeyType &key) const {
  return Hash(key) % num_buckets_secondary_;
}

template <typename KeyType, typename ValueType>
uint64_t ThreadsafeHashmap<KeyType, ValueType>::Size() const {
  return primary_size_.load(std::memory_order_acquire) +
      (state_ == State::kResizing ? secondary_size_.load(std::memory_order_acquire) : 0);
}

template <typename KeyType, typename ValueType>
bool ThreadsafeHashmap<KeyType, ValueType>::Empty() const {
  return 0 == Size();
}

template <typename KeyType, typename ValueType>
void ThreadsafeHashmap<KeyType, ValueType>::Insert(const KeyType &key, const ValueType &value) {
  std::shared_lock<std::shared_timed_mutex> lock(stateupdate_mutex_);

  if (state_ == State::kNormal) {
    if (primary_table_[PrimaryIndex(key)].Insert(key, value))
      primary_size_++;
    if (LoadFactor() > kMaxLoadFactor) {
      lock.unlock();
      ResizingBegin();
    }
  } else {
    if (secondary_table_[SecondaryIndex(key)].Insert(key, value))
      secondary_size_++;
    ContiniousMoving();
    if (0 == primary_size_.load(std::memory_order_acquire)) {
      lock.unlock();
      ResizingDone();
    }
  }
}

template <typename KeyType, typename ValueType>
std::pair<bool, ValueType> ThreadsafeHashmap<KeyType, ValueType>::Lookup(const KeyType &key) const {
  std::shared_lock<std::shared_timed_mutex> lock(stateupdate_mutex_);
  auto result = primary_table_[PrimaryIndex(key)].Lookup(key);
  if (state_ == State::kResizing && !result.first) {
    result = secondary_table_[SecondaryIndex(key)].Lookup(key);
  }

  return result;
}

template <typename KeyType, typename ValueType>
bool ThreadsafeHashmap<KeyType, ValueType>::Remove(const KeyType &key) {
  std::shared_lock<std::shared_timed_mutex> lock(stateupdate_mutex_);
  bool was_removed = primary_table_[PrimaryIndex(key)].Remove(key);
  if (was_removed) {
    primary_size_--;
  }

  if (state_ == State::kNormal)
    return was_removed;

  if (!was_removed) { // we should also try to remove from the second table
    if ((was_removed = secondary_table_[SecondaryIndex(key)].Remove(key)))
      secondary_size_--;
  }
  ContiniousMoving();
  if (0 == primary_size_.load(std::memory_order_acquire)) {
    lock.unlock();
    ResizingDone();
  }
  return was_removed;
}

template <typename KeyType, typename ValueType>
void ThreadsafeHashmap<KeyType, ValueType>::Clear() {
  std::shared_lock<std::shared_timed_mutex> lock(stateupdate_mutex_);
  for (int i = 0; i < num_buckets_primary_; i++)
    primary_table_[i].Clear();
  primary_size_ = 0;

  if (state_ == State::kResizing) {
    for (int i = 0; i < num_buckets_secondary_; i++)
      secondary_table_[i].Clear();
    secondary_size_ = 0;
  }
}

template <typename KeyType, typename ValueType>
void ThreadsafeHashmap<KeyType, ValueType>::ResizingBegin() {
  std::lock_guard<std::shared_timed_mutex> lock(stateupdate_mutex_);
  if (state_ != State::kNormal)
    return;

  num_buckets_secondary_ = static_cast<uint64_t> (num_buckets_primary_ * kIncreaseRate);
  secondary_table_.reset(new Bucket[num_buckets_secondary_]);
  secondary_size_ = 0;
  max_elements_to_move_ = static_cast<uint64_t> (std::sqrt(num_buckets_primary_));
  state_ = State::kResizing;
}

template <typename KeyType, typename ValueType>
void ThreadsafeHashmap<KeyType, ValueType>::ResizingDone() {
  std::lock_guard<std::shared_timed_mutex> lock(stateupdate_mutex_);
  if (state_ != State::kResizing)
    return;

#ifndef NDEBUG
  uint64_t exact_sum = 0;
  for (int i = 0; i < num_buckets_primary_; i++)
    exact_sum += primary_table_[i].Size();
  assert(0 == exact_sum);
#endif

  primary_table_.reset(secondary_table_.release()); // deletes old table and set secondary table ptr to nullptr
  primary_size_ = secondary_size_.load(std::memory_order_acquire);
  secondary_size_ = 0;
  num_buckets_primary_ = num_buckets_secondary_;
  num_buckets_secondary_ = 0;

  state_ = State::kNormal;
}

template <typename KeyType, typename ValueType>
void ThreadsafeHashmap<KeyType, ValueType>::ContiniousMoving() {
  int counter = 0;
  int bucket_id = 0;
  while (counter < max_elements_to_move_ &&
      primary_size_.load(std::memory_order_acquire) > 0 &&
      bucket_id < num_buckets_primary_) {
    auto &bucket = primary_table_[bucket_id];

    while (counter < max_elements_to_move_ && !bucket.Empty()) {
      std::pair<KeyType, ValueType> temp;
      if (kOperationFailed == bucket.PopFront(temp))
        break;
      primary_size_--;
      if (secondary_table_[SecondaryIndex(temp.first)].Insert(std::move(temp)))
        secondary_size_++;
      counter++;
    }
    bucket_id++;
  }
}

} // namespace my_concurrency


#endif //THREADSAFE_HASHMAP_THREADSAFEHASHMAP_H
