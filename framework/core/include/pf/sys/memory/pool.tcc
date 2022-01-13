/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id pool.tcc
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2022 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2022/01/13 14:59
 * @uses Memory pool template.
 *       @refer: https://github.com/cacay/MemoryPool
 */
#ifndef PF_SYS_MEMORY_POOL_TCC_
#define PF_SYS_MEMORY_POOL_TCC_

#include "pf/sys/memory/config.h"

namespace pf_sys {

namespace memory {

template <typename T, size_t BlockSize>
inline typename Pool<T, BlockSize>::size_type
Pool<T, BlockSize>::pad_pointer(data_pointer_t p, size_type align)
const noexcept {
  uintptr_t result = reinterpret_cast<uintptr_t>(p);
  return ((align - result) % align);
}

template <typename T, size_t BlockSize>
Pool<T, BlockSize>::Pool()
noexcept {
  currentblock_ = nullptr;
  currentslot_ = nullptr;
  lastslot_ = nullptr;
  freeslot_ = nullptr;
}

template <typename T, size_t BlockSize>
Pool<T, BlockSize>::Pool(const Pool& memoryPool)
noexcept :
Pool() {}

template <typename T, size_t BlockSize>
Pool<T, BlockSize>::Pool(Pool&& memoryPool)
noexcept {
  currentblock_ = memoryPool.currentblock_;
  memoryPool.currentblock_ = nullptr;
  currentslot_ = memoryPool.currentslot_;
  lastslot_ = memoryPool.lastslot_;
  freeslot_ = memoryPool.freeSlots;
}

template <typename T, size_t BlockSize>
template<class U>
Pool<T, BlockSize>::Pool(const Pool<U>& memoryPool)
noexcept :
Pool() {}

template <typename T, size_t BlockSize>
Pool<T, BlockSize>&
Pool<T, BlockSize>::operator=(Pool&& memoryPool)
noexcept {
  if (this != &memoryPool) {
    std::swap(currentblock_, memoryPool.currentblock_);
    currentslot_ = memoryPool.currentslot_;
    lastslot_ = memoryPool.lastslot_;
    freeslot_ = memoryPool.freeSlots;
  }
  return *this;
}

template <typename T, size_t BlockSize>
Pool<T, BlockSize>::~Pool()
noexcept {
  slot_pointer_t curr = currentblock_;
  while (curr != nullptr) {
    slot_pointer_t prev = curr->next;
    operator delete(reinterpret_cast<void*>(curr));
    curr = prev;
  }
}

template <typename T, size_t BlockSize>
inline typename Pool<T, BlockSize>::pointer
Pool<T, BlockSize>::address(reference x)
const noexcept {
  return &x;
}

template <typename T, size_t BlockSize>
inline typename Pool<T, BlockSize>::const_pointer
Pool<T, BlockSize>::address(const_reference x)
const noexcept {
  return &x;
}

template <typename T, size_t BlockSize>
void
Pool<T, BlockSize>::allocate_block() {
  // Allocate space for the new block and store a pointer to the previous one
  data_pointer_t newBlock = reinterpret_cast<data_pointer_t>
                           (operator new(BlockSize));
  reinterpret_cast<slot_pointer_t>(newBlock)->next = currentblock_;
  currentblock_ = reinterpret_cast<slot_pointer_t>(newBlock);
  // Pad block body to staisfy the alignment requirements for elements
  data_pointer_t body = newBlock + sizeof(slot_pointer_t);
  size_type body_padding = pad_pointer(body, alignof(slot_type_t));
  currentslot_ = reinterpret_cast<slot_pointer_t>(body + body_padding);
  lastslot_ = reinterpret_cast<slot_pointer_t>
              (newBlock + BlockSize - sizeof(slot_type_t) + 1);
}

template <typename T, size_t BlockSize>
inline typename Pool<T, BlockSize>::pointer
Pool<T, BlockSize>::allocate(size_type n, const_pointer hint) {
  UNUSED(n);
  UNUSED(hint);
  if (freeslot_ != nullptr) {
    pointer result = reinterpret_cast<pointer>(freeslot_);
    freeslot_ = freeslot_->next;
    return result;
  } else {
    if (currentslot_ >= lastslot_)
      allocate_block();
    return reinterpret_cast<pointer>(currentslot_++);
  }
}

template <typename T, size_t BlockSize>
inline void
Pool<T, BlockSize>::deallocate(pointer p, size_type n) {
  if (p != nullptr) {
    reinterpret_cast<slot_pointer_t>(p)->next = freeslot_;
    freeslot_ = reinterpret_cast<slot_pointer_t>(p);
  }
}

template <typename T, size_t BlockSize>
inline typename Pool<T, BlockSize>::size_type
Pool<T, BlockSize>::max_size()
const noexcept {
  size_type maxBlocks = -1 / BlockSize;
  return (BlockSize - sizeof(data_pointer_t)) / sizeof(slot_type_t) * maxBlocks;
}

template <typename T, size_t BlockSize>
template <class U, class... Args>
inline void
Pool<T, BlockSize>::construct(U* p, Args&&... args) {
  new (p) U (std::forward<Args>(args)...);
}

template <typename T, size_t BlockSize>
template <class U>
inline void
Pool<T, BlockSize>::destroy(U* p) {
  p->~U();
}

template <typename T, size_t BlockSize>
template <class... Args>
inline typename Pool<T, BlockSize>::pointer
Pool<T, BlockSize>::new_element(Args&&... args) {
  pointer result = allocate();
  construct<value_type>(result, std::forward<Args>(args)...);
  return result;
}

template <typename T, size_t BlockSize>
inline void
Pool<T, BlockSize>::delete_element(pointer p) {
  if (p != nullptr) {
    p->~value_type();
    deallocate(p);
  }
}

} // namespace memory

} // namespace pf_sys

#endif // PF_SYS_MEMORY_POOL_TCC_ 
