#ifndef THREADSAFE_HASHMAP_THREADSAFEHASHMAP_H
#define THREADSAFE_HASHMAP_THREADSAFEHASHMAP_H

#include <atomic>
#include <functional> // hash
#include <memory>
#include <shared_mutex>

#include "../src/concurrent_linked_list.h"

namespace my_concurrency {

template <typename KeyType, typename ValueType>
class ThreadsafeHashmap {
 public:
  ThreadsafeHashmap(uint64_t num_buckets = 100);

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

  double LoadFactor() const;

  uint64_t PrimaryIndex(const KeyType &key);
  uint64_t SecondaryIndex(const KeyType &key);

  void ResizingBegin();
  void ResizingDone();

  typedef internals::ConcurrentLinkedList<KeyType, ValueType> Bucket;
  static const double kIncreaseRate = 2.0;

  uint64_t num_buckets_primary_;
  uint64_t num_buckets_secondary_;
  std::unique_ptr<Bucket[]> primary_table_;
  std::unique_ptr<Bucket[]> secondary_table_;
  std::atomic_ullong primary_size_;
  std::atomic_ullong secondary_size_;
  std::hash<KeyType> hash_;

  std::atomic<State> state_;

};

template <typename KeyType, typename ValueType>
ThreadsafeHashmap<KeyType, ValueType>::ThreadsafeHashmap(uint64_t num_buckets)
    : num_buckets_primary_(num_buckets),
    primary_size_(0),
    primary_table_(new Bucket[num_buckets]) {}

template <typename KeyType, typename ValueType>
double ThreadsafeHashmap::LoadFactor() const {
  return primary_size_.load(std::memory_order_seq_cst) / double(num_buckets_primary_);
}

template <typename KeyType, typename ValueType>
uint64_t ThreadsafeHashmap::PrimaryIndex(const KeyType &key) {
  return hash_(key) % num_buckets_primary_;
}

template <typename KeyType, typename ValueType>
uint64_t ThreadsafeHashmap::SecondaryIndex(const KeyType &key) {
  return hash_(key) % num_buckets_secondary_;
}
} // namespace my_concurrency


#endif //THREADSAFE_HASHMAP_THREADSAFEHASHMAP_H
