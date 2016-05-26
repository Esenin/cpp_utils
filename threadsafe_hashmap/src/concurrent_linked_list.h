#ifndef THREADSAFE_HASHMAP_LINKEDLIST_H
#define THREADSAFE_HASHMAP_LINKEDLIST_H

#include <inttypes.h>
#include <iterator>     // iterator
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <utility> // pair

namespace my_concurrency {
namespace internals {

/// @brief Many readers - single writer  linked list with serialized (blocking!) iterator
/// @tparam ValueType should have default constructor in order to lookup non-existing elements
template<typename KeyType, typename ValueType>
class ConcurrentLinkedList {
 public:
  ConcurrentLinkedList() { }
  ~ConcurrentLinkedList() {
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
  ///               second part is a payload
  std::pair<bool, ValueType> Lookup(const KeyType &key) const;

  /// @brief Remove all the elements in the list
  void Clear();
  /// @brief check if the list is empty
  bool Empty() const;

  /// @brief const forward iterator
  class ListIterator;

  typedef ListIterator iterator;
  iterator Begin() const;
  iterator End() const;

  void PopFront(std::pair<KeyType, ValueType> &result);

  constexpr static bool kOperationSuccess = true;
  constexpr static bool kOperationFailed = false;
 private:
  struct ListElement {
    KeyType key;
    ValueType value;
    ListElement *next = nullptr;

    ListElement(const KeyType &key, const ValueType &value) : key(key), value(value) { }
    ListElement(KeyType &&key, ValueType &&value) : key(key), value(value) { }
  };

  /// @brief places new node into the list. Releases the pointer or moves the object
  /// @return true if new element was inserted, false if a node was overwritten
  bool InsertListElement(std::unique_ptr<ListElement> &node);

  mutable std::shared_timed_mutex mutex_;
  ListElement *head_ = nullptr;
  ListElement *tail_ = nullptr;
  uint64_t size_ = 0;
};

template<typename KeyType, typename ValueType>
uint64_t ConcurrentLinkedList<KeyType, ValueType>::Size() const {
  return size_;
}

template<typename KeyType, typename ValueType>
void ConcurrentLinkedList<KeyType, ValueType>::Clear() {
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);
  while (head_ != nullptr) {
    auto temp = head_;
    head_ = head_->next;
    delete temp;
  }
  size_ = 0;
}

template<typename KeyType, typename ValueType>
std::pair<bool, ValueType> ConcurrentLinkedList<KeyType, ValueType>::Lookup(const KeyType &key) const {
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);

  auto temp = head_;
  while (temp != nullptr)
    if (temp->key == key)
      return {true, temp->value};
    else
      temp = temp->next;

  return {false, ValueType()};
}

template<typename KeyType, typename ValueType>
bool ConcurrentLinkedList<KeyType, ValueType>::Remove(const KeyType &key) {
  if (Empty())
    return kOperationFailed;
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);

  auto temp = head_;
  if (head_->key == key) {
    head_ = head_->next;
    delete temp;
    if (0 == --size_)
      tail_ = nullptr;
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
  if (target_node == tail_)
    tail_ = temp;
  temp->next = target_node->next;
  delete target_node;
  size_--;
  return kOperationSuccess;
}

template<typename KeyType, typename ValueType>
void ConcurrentLinkedList<KeyType, ValueType>::PopFront(std::pair<KeyType, ValueType> &result) {
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);
  if (Empty())
    throw std::out_of_range("Trying to pop from empty list");
  result.first = std::move(head_->key);
  result.second = std::move(head_->value);
  auto temp = head_;
  head_ = head_->next;
  delete temp;
  if (0 == --size_)
    tail_ = nullptr;
}

template<typename KeyType, typename ValueType>
bool ConcurrentLinkedList<KeyType, ValueType>::Insert(const KeyType &key, const ValueType &value) {
  auto new_node = std::unique_ptr<ListElement>(new ListElement(key, value));
  return InsertListElement(new_node);
}

template<typename KeyType, typename ValueType>
bool ConcurrentLinkedList<KeyType, ValueType>::Insert(KeyType &&key, ValueType &&value) {
  auto new_node = std::unique_ptr<ListElement>(new ListElement(std::forward<KeyType>(key),
                                                               std::forward<ValueType>(value)));
  return InsertListElement(new_node);
}

template<typename KeyType, typename ValueType>
bool ConcurrentLinkedList<KeyType, ValueType>::Insert(std::pair<KeyType, ValueType> &&kv_pair) {
  return Insert(std::move(kv_pair.first), std::move(kv_pair.second));
};

template<typename KeyType, typename ValueType>
bool ConcurrentLinkedList<KeyType, ValueType>::InsertListElement(std::unique_ptr<ListElement> &node) {
  const bool kWasNewElementCreated = true;
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);
  if (Empty()) {
    head_ = node.release();
    tail_ = head_;
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

  tail_->next = node.release();
  tail_ = tail_->next;
  size_++;
  return kWasNewElementCreated;
}

template<typename KeyType, typename ValueType>
bool ConcurrentLinkedList<KeyType, ValueType>::Empty() const {
  return 0 == size_;
}

template<typename KeyType, typename ValueType>
class ConcurrentLinkedList<KeyType, ValueType>::ListIterator : public std::iterator<std::forward_iterator_tag, KeyType> {
 public:
  ListIterator() : node_ptr_(nullptr) {}
  ListIterator(const ListIterator &) = delete;
  ListIterator(ListIterator &&rhs) {
    *this = std::forward<ListIterator>(rhs);
  }
  ListIterator(const ConcurrentLinkedList & list) : node_ptr_(list.head_), lock_(list.mutex_, std::defer_lock) {
    if (!list.Empty())
      lock_.lock();
  }

  ListIterator& operator++() {
    if (node_ptr_) {
      node_ptr_ = node_ptr_->next;
      if (nullptr == node_ptr_ && lock_.owns_lock()) {
        lock_.unlock();
        lock_.release();
      }
    }
    return *this;
  }

  bool operator==(const ListIterator &rhs) { return node_ptr_ == rhs.node_ptr_; }
  bool operator!=(const ListIterator &rhs) { return node_ptr_ != rhs.node_ptr_; }
  std::pair<KeyType&, ValueType&> operator*() { return {node_ptr_->key, node_ptr_->value}; }
  std::pair<KeyType&, ValueType&> operator->() { return {node_ptr_->key, node_ptr_->value}; }

  ListIterator& operator=(const ListIterator &) = delete;
  ListIterator& operator=(ListIterator &&rhs) {
    lock_ = std::move(rhs.lock_);
    node_ptr_ = rhs.node_ptr_;
    rhs.node_ptr_ = nullptr;
  }

 private:
  ListElement *node_ptr_;
  std::unique_lock<std::shared_timed_mutex> lock_; // synchronious iterator
};

template<typename KeyType, typename ValueType>
typename ConcurrentLinkedList<KeyType, ValueType>::iterator ConcurrentLinkedList<KeyType, ValueType>::Begin() const {
  return ListIterator(*this);
}

template<typename KeyType, typename ValueType>
typename ConcurrentLinkedList<KeyType, ValueType>::iterator ConcurrentLinkedList<KeyType, ValueType>::End() const {
  return ListIterator();
}

} // namespace internals
} // namespace my_concurrency

#endif //THREADSAFE_HASHMAP_LINKEDLIST_H

