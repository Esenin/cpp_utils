#ifndef THREADSAFE_HASHMAP_LINKEDLIST_H
#define THREADSAFE_HASHMAP_LINKEDLIST_H

#include <inttypes.h>
#include <iterator>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <utility> // pair

namespace my_concurrency {
namespace internals {

/// @brief Many readers - single writer bucket based on single linked list
/// @tparam ValueType should have default constructor in order to lookup non-existing elements
template <typename KeyType, typename ValueType>
class Bucket {
 public:
  Bucket() = default;

  /// @brief snapshot copy: requires full lock
  Bucket(const Bucket &rhs);
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
  /// @warning thread-UNsafe
  class ListIterator;
  typedef ListIterator iterator;

  /// @return forward iterator
  /// @warning thread-UNsafe
  iterator Begin() const;
  iterator End() const;

  /// @brief Pops first element from the list to provided object
  /// @param result front element moves to this container
  /// @return true if successful, false in case the list is empty
  bool PopFront(std::pair<KeyType, ValueType> &result);

  /// @brief makes a snapshot full copy of the other bucket
  Bucket& operator=(const Bucket &rhs);

  constexpr static bool kOperationSuccess = true;
  constexpr static bool kOperationFailed = false;
 private:
  struct ListNode {
    KeyType key;
    ValueType value;
    ListNode *next = nullptr;

    ListNode(const KeyType &key, const ValueType &value) : key(key), value(value) { }
    ListNode(KeyType &&key, ValueType &&value) : key(key), value(value) { }
    ListNode(const ListNode &) = delete;
    ListNode &operator=(const ListNode &) = delete;
  };

  /// @brief places new node into the list. Releases the pointer or moves the object
  /// @return true if new element was inserted, false if a node was overwritten
  bool InsertListElement(std::unique_ptr<ListNode> &node);

  mutable std::shared_timed_mutex mutex_;
  ListNode *head_ = nullptr;
  uint64_t size_ = 0;
};

template <typename KeyType, typename ValueType>
Bucket<KeyType, ValueType>::Bucket(const Bucket &rhs) {
  *this = rhs;
}

template <typename KeyType, typename ValueType>
Bucket<KeyType, ValueType> &Bucket<KeyType, ValueType>::operator=(const Bucket &rhs) {
  std::lock_guard<std::shared_timed_mutex>(rhs.mutex_);
  for (auto iter = rhs.Begin(); iter != rhs.End(); ++iter) {
    auto new_node = new ListNode((*iter).first, (*iter).second);
    new_node->next = head_;
    head_ = new_node;
    size_++;
  }

  return *this;
}

template <typename KeyType, typename ValueType>
uint64_t Bucket<KeyType, ValueType>::Size() const {
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);
  return size_;
}

template <typename KeyType, typename ValueType>
void Bucket<KeyType, ValueType>::Clear() {
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);
  while (head_ != nullptr) {
    auto temp = head_;
    head_ = head_->next;
    delete temp;
  }
  size_ = 0;
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
  if (0 == size_)
    return kOperationFailed;

  auto temp = head_;
  if (head_->key == key) {
    head_ = head_->next;
    delete temp;
    --size_;
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
  if (0 == size_)
    return kOperationFailed;
  result.first = std::move(head_->key);
  result.second = std::move(head_->value);
  auto temp = head_;
  head_ = head_->next;
  delete temp;
  --size_;
  return kOperationSuccess;
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
  if (0 == size_) {
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
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);
  return 0 == size_;
}

template <typename KeyType, typename ValueType>
class Bucket<KeyType, ValueType>::ListIterator: public std::iterator<std::forward_iterator_tag, KeyType> {
 public:
  ListIterator() : node_ptr_(nullptr) { }
  ListIterator(const ListIterator &rhs) : node_ptr_(rhs.node_ptr_) { }
  ListIterator(const Bucket &list) : node_ptr_(list.head_) { }

  ListIterator &operator++() {
    if (node_ptr_)
      node_ptr_ = node_ptr_->next;
    return *this;
  }

  bool operator==(const ListIterator &rhs) { return node_ptr_ == rhs.node_ptr_; }
  bool operator!=(const ListIterator &rhs) { return node_ptr_ != rhs.node_ptr_; }
  std::pair<KeyType &, ValueType &> operator*() { return {node_ptr_->key, node_ptr_->value}; }

  ListIterator &operator=(const ListIterator &rhs) {
    node_ptr_ = rhs.node_ptr_;
    return *this;
  }

 private:
  ListNode *node_ptr_;
};

template <typename KeyType, typename ValueType>
typename Bucket<KeyType, ValueType>::iterator Bucket<KeyType, ValueType>::Begin() const {
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);
  return ListIterator(*this);
}

template <typename KeyType, typename ValueType>
typename Bucket<KeyType, ValueType>::iterator Bucket<KeyType, ValueType>::End() const {
  return ListIterator();
}

} // namespace internals
} // namespace my_concurrency

#endif //THREADSAFE_HASHMAP_LINKEDLIST_H

