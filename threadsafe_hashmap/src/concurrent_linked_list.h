#ifndef THREADSAFE_HASHMAP_LINKEDLIST_H
#define THREADSAFE_HASHMAP_LINKEDLIST_H

#include <inttypes.h>
#include <iterator>     // iterator
#include <mutex>
#include <shared_mutex>

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
  void Insert(const KeyType &key, const ValueType &value);

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

  constexpr static bool kOperationSuccess = true;
  constexpr static bool kOperationFailed = false;

  /// @brief const forward iterator
  class ListIterator;

  typedef ListIterator iterator;
  iterator begin() const;
  iterator end() const;
 private:
  struct ListElement {
    KeyType key;
    ValueType value;
    ListElement *next;

    ListElement(const KeyType &key, const ValueType &value) : key(key), value(value), next(nullptr) { }
  };

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
void ConcurrentLinkedList<KeyType, ValueType>::Insert(const KeyType &key, const ValueType &value) {
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);
  if (Empty()) {
    head_ = new ListElement(key, value);
    tail_ = head_;
    size_++;
    return;
  }

  auto temp = head_;
  while (temp != nullptr) {
    if (temp->key == key) {
      temp->value = value;
      return;
    } else {
      temp = temp->next;
    }
  }

  tail_->next = new ListElement(key, value);
  tail_ = tail_->next;
  size_++;
}

template<typename KeyType, typename ValueType>
bool ConcurrentLinkedList<KeyType, ValueType>::Empty() const {
  return 0 == size_;
}

template<typename KeyType, typename ValueType>
class ConcurrentLinkedList<KeyType, ValueType>::ListIterator : public std::iterator<std::forward_iterator_tag, KeyType> {
 public:
  ListIterator() : node_ptr_(nullptr) {}
  ListIterator(const ConcurrentLinkedList & list) : node_ptr_(list.head_), lock_(list.mutex_) {}

  ListIterator& operator++() {
    if (node_ptr_)
      node_ptr_ = node_ptr_->next;
    return *this;
  }

  bool operator==(const ListIterator &rhs) { return node_ptr_ == rhs.node_ptr_; }
  bool operator!=(const ListIterator &rhs) { return node_ptr_ != rhs.node_ptr_; }
  std::pair<KeyType&, ValueType&> operator*() { return {node_ptr_->key, node_ptr_->value}; }

 private:
  ListElement *node_ptr_;
  std::unique_lock<std::shared_timed_mutex> lock_; // synchronious iterator
};

template<typename KeyType, typename ValueType>
typename ConcurrentLinkedList<KeyType, ValueType>::iterator ConcurrentLinkedList<KeyType, ValueType>::begin() const {
  return ListIterator(*this);
}

template<typename KeyType, typename ValueType>
typename ConcurrentLinkedList<KeyType, ValueType>::iterator ConcurrentLinkedList<KeyType, ValueType>::end() const {
  return ListIterator();
}
} // namespace internals
} // namespace my_concurrency

#endif //THREADSAFE_HASHMAP_LINKEDLIST_H

