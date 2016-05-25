#ifndef THREADSAFE_HASHMAP_LINKEDLIST_H
#define THREADSAFE_HASHMAP_LINKEDLIST_H

#include <inttypes.h>
#include <iterator>     // iterator

namespace my_concurrency {
namespace internals {

/// @brief Single-linked list with iterator
template<typename Type>
class LinkedList {
 public:
  LinkedList() { }
  ~LinkedList() {
    Clear();
  }

  uint64_t Size() const;

  /// @brief add element to list
  /// @param value insert to the list
  /// @return true in case successful appending, false if the element already exists in the list
  bool TryAppend(const Type &value);

  /// @brief remove element from the list
  /// @param value element to delete
  /// @return true in case successful removal, false in case no such value in the list
  bool TryRemove(const Type &value);

  /// @brief check if the value exists in the list
  bool Lookup(const Type &value) const;
  /// @brief Remove all the elements in the list
  void Clear();
  /// @brief check if the list is empty
  bool Empty() const;

  constexpr static bool kOperationSuccess = true;
  constexpr static bool kOperationFailed = false;

  /// @brief const forward iterator
  class ListIterator;

  typedef ListIterator const_iterator;
  const_iterator begin() const;
  const_iterator end() const;
 private:
  struct ListElement {
    Type value;
    ListElement *next;

    ListElement(const Type &value) : value(value), next(nullptr) { }
  };
  ListElement *head_ = nullptr;
  ListElement *tail_ = nullptr;
  uint64_t size_ = 0;
};

template<typename Type>
uint64_t LinkedList<Type>::Size() const {
  return size_;
}

template<typename Type>
void LinkedList<Type>::Clear() {
  while (head_ != nullptr) {
    auto temp = head_;
    head_ = head_->next;
    delete temp;
  }
  size_ = 0;
}

template<typename Type>
bool LinkedList<Type>::Lookup(const Type &value) const {
  auto temp = head_;

  while (temp != nullptr)
    if (temp->value == value)
      return true;
    else
      temp = temp->next;

  return false;
}

template<typename Type>
bool LinkedList<Type>::TryRemove(const Type &value) {
  auto temp = head_;
  if (head_->value == value) {
    head_ = head_->next;
    delete temp;
    if (0 == --size_)
      tail_ = nullptr;
    return kOperationSuccess;
  }

  while (temp != nullptr) {
    if (temp->next != nullptr && temp->next->value == value)
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

template<typename Type>
bool LinkedList<Type>::TryAppend(const Type &value) {
  if (Empty()) {
    head_ = new ListElement(value);
    tail_ = head_;
    size_++;
    return kOperationSuccess;
  }

  auto temp = head_;

  while (temp != nullptr) {
    if (temp->value == value)
      return kOperationFailed;
    else
      temp = temp->next;
  }

  tail_->next = new ListElement(value);
  tail_ = tail_->next;
  size_++;
  return kOperationSuccess;
}

template<typename Type>
bool LinkedList<Type>::Empty() const {
  return 0 == size_;
}

template<typename Type>
class LinkedList<Type>::ListIterator : public std::iterator<std::forward_iterator_tag, Type> {
 public:
  ListIterator() : node_ptr_(nullptr) {}
  ListIterator(const LinkedList& list) : node_ptr_(list.head_) {}

  ListIterator& operator++() {
    if (node_ptr_)
      node_ptr_ = node_ptr_->next;
    return *this;
  }

  bool operator==(const ListIterator &rhs) { return node_ptr_ == rhs.node_ptr_; }
  bool operator!=(const ListIterator &rhs) { return node_ptr_ != rhs.node_ptr_; }
  const Type& operator*() { return node_ptr_->value; }

 private:
  ListElement *node_ptr_;
};

template<typename Type>
typename LinkedList<Type>::const_iterator LinkedList<Type>::begin() const {
  return ListIterator(*this);
}

template<typename Type>
typename LinkedList<Type>::const_iterator LinkedList<Type>::end() const {
  return ListIterator();
}
} // namespace internals
} // namespace my_concurrency

#endif //THREADSAFE_HASHMAP_LINKEDLIST_H

