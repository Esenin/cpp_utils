#ifndef THREADSAFE_HASHMAP_LINKEDLIST_H
#define THREADSAFE_HASHMAP_LINKEDLIST_H

#include <atomic>
#include <functional>
#include <inttypes.h>
#include <iterator>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <utility> // pair

#include "helpers.h"

namespace my_concurrency {
namespace internals {

/// @brief Many readers - single writer bucket based on single linked list
/// @tparam ValueType should have default constructor in order to lookup non-existing elements
template <typename KeyType, typename ValueType>
class Bucket {
 public:
  Bucket() : size_(0) { }

  /// @brief snapshot copy: requires full lock
  Bucket(const Bucket &rhs);

  Bucket(Bucket &&rhs);
  ~Bucket() {
    Clear();
  }

  uint64_t Size() const;

  /// @brief add key-value pair to the list. Rewrites value in case the key already exists
  /// @return true if new element was inserted, false if a node was overwritten
  bool Insert(const KeyType &key, const ValueType &value);
  bool Insert(KeyType &&key, ValueType &&value);
  bool Insert(std::pair<KeyType, ValueType> &&kv_pair);

  /// @brief remove element from the list
  /// @param key of the element to delete
  /// @return true in case successful removal, false in case no such key in the list
  bool Remove(const KeyType &key);

  /// @brief find the element by key
  /// @return pair: first part is true if element with such key exists in the list, false otherwise
  ///               second part is a value
  std::pair<bool, ValueType> Lookup(const KeyType &key) const;

  /// @brief Remove all elements in the list
  void Clear();
  /// @brief check if the list is empty
  bool Empty() const;

  /// @brief forward iterator
  /// @warning exclusively synchronized
  class ListIterator;
  typedef ListIterator iterator;

  /// @brief serialized iterator: exclusively locks the bucket
  /// @return forward iterator
  iterator BeginSync() const;
  iterator End() const;

  /// @brief iterator which is NOT threadsafe
  iterator Begin() const;

  /// @brief Pops first element from the list to provided object
  /// @param result front element moves to this container
  /// @return true if successful, false in case the list is empty
  bool PopFront(std::pair<KeyType, ValueType> &result);

  /// @brief makes a snapshot full copy of the other bucket
  Bucket &operator=(const Bucket &rhs);

  void Swap(Bucket &rhs);

  /// @brief Moves all of the items to different buckets obtained by dest function
  /// @param dest function returns appropriate bucket according to the provided key
  /// @returns number of items were migrated
  uint64_t MigrateTo(std::function<Bucket &(const KeyType &)> dest);

  constexpr static bool kOperationSuccess = true;
  constexpr static bool kOperationFailed = false;
 private:
  struct ListNode {
    KeyType key;
    ValueType value;
    ListNode *next = nullptr;

    ListNode(const KeyType &key, const ValueType &value) : key(key), value(value) { }
    ListNode(KeyType &&key, ValueType &&value) : key(std::move(key)), value(std::move(value)) { }
    ListNode(const ListNode &) = delete;
    ListNode &operator=(const ListNode &) = delete;
  };

  /// @brief places new node into the list. Releases the pointer or moves the object
  /// @return true if new element was inserted, false if a node was overwritten
  bool InsertListElement(std::unique_ptr<ListNode> &node);

  mutable std::shared_timed_mutex mutex_;
  ListNode *head_ = nullptr;
  std::atomic_ullong size_;
};

template <typename KeyType, typename ValueType>
Bucket<KeyType, ValueType>::Bucket(const Bucket &rhs) {
  *this = rhs;
}

template <typename KeyType, typename ValueType>
Bucket<KeyType, ValueType> &Bucket<KeyType, ValueType>::operator=(const Bucket &rhs) {
  Clear();
  for (auto iter = rhs.BeginSync(); iter != rhs.End(); ++iter) {
    auto new_node = new ListNode((*iter).first, (*iter).second);
    new_node->next = head_;
    head_ = new_node;
    size_++;
  }

  return *this;
}

template <typename KeyType, typename ValueType>
Bucket<KeyType, ValueType>::Bucket(Bucket &&rhs) {
  std::lock_guard<std::shared_timed_mutex> lock(rhs.mutex_);
  head_ = rhs.head_;
  rhs.head_ = nullptr;
  size_.store(rhs.size_.load(std::memory_order_acquire), std::memory_order_release);
  rhs.size_.store(0, std::memory_order_release);
}

template <typename KeyType, typename ValueType>
void Bucket<KeyType, ValueType>::Swap(Bucket &rhs) {
  std::lock_guard<std::shared_timed_mutex> rhs_lock(rhs.mutex_);
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);
  std::swap(head_, rhs.head_);
  auto rhs_size = rhs.size_.load(std::memory_order_acquire);
  rhs.size_.store(size_.load(std::memory_order_acquire), std::memory_order_release);
  size_.store(rhs_size, std::memory_order_release);
}


template <typename KeyType, typename ValueType>
uint64_t Bucket<KeyType, ValueType>::Size() const {
  return size_.load(std::memory_order_acquire);
}

template <typename KeyType, typename ValueType>
void Bucket<KeyType, ValueType>::Clear() {
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);
  while (head_ != nullptr) {
    auto temp = head_;
    head_ = head_->next;
    delete temp;
  }
  size_.store(0, std::memory_order_release);
}

template <typename KeyType, typename ValueType>
std::pair<bool, ValueType> Bucket<KeyType, ValueType>::Lookup(const KeyType &key) const {
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);

  auto temp = head_;
  while (temp != nullptr)
    if (temp->key == key)
      return {true, temp->value};
    else
      temp = temp->next;

  return {false, ValueType()};
}

template <typename KeyType, typename ValueType>
bool Bucket<KeyType, ValueType>::Remove(const KeyType &key) {
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);
  if (0 == size_.load(std::memory_order_acquire))
    return kOperationFailed;

  auto temp = head_;
  if (head_->key == key) {
    head_ = head_->next;
    delete temp;
    size_--;
    return kOperationSuccess;
  }

  while (temp != nullptr) {
    if (temp->next != nullptr && temp->next->key == key)
      break;
    else
      temp = temp->next;
  }

  if (temp == nullptr)
    return kOperationFailed;

  auto target_node = temp->next;
  temp->next = target_node->next;
  delete target_node;
  size_--;
  return kOperationSuccess;
}

template <typename KeyType, typename ValueType>
bool Bucket<KeyType, ValueType>::PopFront(std::pair<KeyType, ValueType> &result) {
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);
  if (0 == size_.load(std::memory_order_acquire))
    return kOperationFailed;
  result.first = std::move(head_->key);
  result.second = std::move(head_->value);
  auto temp = head_;
  head_ = head_->next;
  delete temp;
  size_--;
  return kOperationSuccess;
}

template <typename KeyType, typename ValueType>
uint64_t Bucket<KeyType, ValueType>::MigrateTo(std::function<Bucket &(const KeyType &)> dest) {
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);
  auto node = head_;
  std::unique_ptr<ListNode> uniq_node(node);
  while (node != nullptr) {
    auto &bucket = dest(node->key);
    node = node->next;
    uniq_node->next = nullptr;
    bucket.InsertListElement(uniq_node);
    uniq_node.reset(node);
  }
  head_ = nullptr;
  const uint64_t kNumItems = size_.load(std::memory_order_acquire);
  size_.store(0, std::memory_order_release);
  return kNumItems;
}

template <typename KeyType, typename ValueType>
bool Bucket<KeyType, ValueType>::Insert(const KeyType &key, const ValueType &value) {
  auto new_node = std::unique_ptr<ListNode>(new ListNode(key, value));
  return InsertListElement(new_node);
}

template <typename KeyType, typename ValueType>
bool Bucket<KeyType, ValueType>::Insert(KeyType &&key, ValueType &&value) {
  auto new_node = std::unique_ptr<ListNode>(new ListNode(std::forward<KeyType>(key),
                                                         std::forward<ValueType>(value)));
  return InsertListElement(new_node);
}

template <typename KeyType, typename ValueType>
bool Bucket<KeyType, ValueType>::Insert(std::pair<KeyType, ValueType> &&kv_pair) {
  return Insert(std::move(kv_pair.first), std::move(kv_pair.second));
};

template <typename KeyType, typename ValueType>
bool Bucket<KeyType, ValueType>::InsertListElement(std::unique_ptr<ListNode> &node) {
  const bool kWasNewElementCreated = true;
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);
  if (0 == size_.load(std::memory_order_acquire)) {
    head_ = node.release();
    size_++;
    return kWasNewElementCreated;
  }

  auto temp = head_;
  while (temp != nullptr) {
    if (temp->key == node->key) {
      temp->value = std::move(node->value);
      return !kWasNewElementCreated;
    } else {
      temp = temp->next;
    }
  }

  temp = head_;
  head_ = node.release();
  head_->next = temp;
  size_++;
  return kWasNewElementCreated;
}

template <typename KeyType, typename ValueType>
bool Bucket<KeyType, ValueType>::Empty() const {
  return 0 == size_.load(std::memory_order_acquire);
}

template <typename KeyType, typename ValueType>
class Bucket<KeyType, ValueType>::ListIterator: public std::iterator<std::forward_iterator_tag, KeyType> {
 public:
  ListIterator() : node_ptr_(nullptr) { }
  ListIterator(ListIterator &&rhs) : node_ptr_(rhs.node_ptr_), lock_(std::move(rhs.lock_)) {
    rhs.node_ptr_ = nullptr;
  }
  ListIterator(const Bucket &list, bool do_lock = true) : node_ptr_(list.head_), lock_(list.mutex_, std::defer_lock) {
    if (!list.Empty() && do_lock)
      lock_.lock();
  }

  ListIterator &operator++() {
    if (node_ptr_)
      node_ptr_ = node_ptr_->next;
    if (nullptr == node_ptr_ && lock_.owns_lock()) {
      lock_.unlock();
      lock_.release();
    }
    return *this;
  }

  bool operator==(const ListIterator &rhs) { return node_ptr_ == rhs.node_ptr_; }
  bool operator!=(const ListIterator &rhs) { return node_ptr_ != rhs.node_ptr_; }
  std::pair<KeyType &, ValueType &> operator*() { return {node_ptr_->key, node_ptr_->value}; }

  ListIterator &operator=(ListIterator &&rhs) {
    node_ptr_ = rhs.node_ptr_;
    if (lock_.owns_lock())
      lock_.unlock();
    lock_ = std::move(rhs.lock_);
    rhs.node_ptr_ = nullptr;
    return *this;
  }

 private:
  ListNode *node_ptr_;
  std::unique_lock<std::shared_timed_mutex> lock_;
};

template <typename KeyType, typename ValueType>
typename Bucket<KeyType, ValueType>::iterator Bucket<KeyType, ValueType>::BeginSync() const {
  const bool kLockMutex = true;
  return ListIterator(*this, kLockMutex);
}

template <typename KeyType, typename ValueType>
typename Bucket<KeyType, ValueType>::iterator Bucket<KeyType, ValueType>::Begin() const {
  const bool kLockMutex = false;
  return ListIterator(*this, kLockMutex);
}

template <typename KeyType, typename ValueType>
typename Bucket<KeyType, ValueType>::iterator Bucket<KeyType, ValueType>::End() const {
  return ListIterator();
}

} // namespace internals
} // namespace my_concurrency

#endif //THREADSAFE_HASHMAP_LINKEDLIST_H

