#ifndef THREADSAFE_HASHMAP_LINKEDLIST_H
#define THREADSAFE_HASHMAP_LINKEDLIST_H

#include <inttypes.h>

template<typename Type>
class LinkedList {
 public:
  LinkedList() {}
  ~LinkedList() {
    Clear();
  }

  uint64_t Size() const;
  bool TryAppend(const Type &value);
  bool TryRemove(const Type &value);
  bool Lookup(const Type &value) const;
  void Clear();
  bool Empty() const;

  constexpr static bool kOperationSuccess = true;
  constexpr static bool kOperationFailed = false;
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

#endif //THREADSAFE_HASHMAP_LINKEDLIST_H

