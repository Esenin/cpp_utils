#ifndef THREADSAFE_HASHMAP_BUCKET_H
#define THREADSAFE_HASHMAP_BUCKET_H

#include <mutex>
#include <shared_mutex>


namespace my_concurrency {
namespace impl {

template <typename Type>
class Bucket {
 public:
  void Insert(const Type &value);

 private:
  struct ListNode {
    Type value;
    ListNode *next;

    ListNode(const Type &value) : value(value), next(nullptr) {}
  };

  mutable std::shared_timed_mutex mutex_;
};

template <typename Type>
void Bucket<Type>::Insert(const Type &value) {
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);


}
} // namespace impl
} // namespace my_concurrency


#endif //THREADSAFE_HASHMAP_BUCKET_H
