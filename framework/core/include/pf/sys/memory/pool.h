/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id pool.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2022 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2022/01/13 14:52
 * @uses The memory pool implement class.
 *       @refer: https://github.com/cacay/MemoryPool
 */

#ifndef PF_SYS_MEMORY_POOL_H_
#define PF_SYS_MEMORY_POOL_H_

#include "pf/sys/memory/config.h"

namespace pf_sys {

namespace memory {

template <typename T, size_t BlockSize = 4096>
class Pool {
 public:
   /* Member types */
   typedef T               value_type;
   typedef T*              pointer;
   typedef T&              reference;
   typedef const T*        const_pointer;
   typedef const T&        const_reference;
   typedef size_t          size_type;
   typedef ptrdiff_t       difference_type;
   typedef std::false_type propagate_on_container_copy_assignment;
   typedef std::true_type  propagate_on_container_move_assignment;
   typedef std::true_type  propagate_on_container_swap;

   template <typename U> struct rebind {
     typedef Pool<U> other;
   };

   /* Member functions */
   Pool() noexcept;
   Pool(const Pool &memory_pool) noexcept;
   Pool(Pool &&memory_pool) noexcept;
   template <class U> Pool(const Pool<U> &memory_pool) noexcept;

   ~Pool() noexcept;

   Pool& operator=(const Pool &memory_pool) = delete;
   Pool& operator=(Pool &&memory_pool) noexcept;

   pointer address(reference x) const noexcept;
   const_pointer address(const_reference x) const noexcept;

   // Can only allocate one object at a time. n and hint are ignored
   pointer allocate(size_type n = 1, const_pointer hint = 0);
   void deallocate(pointer p, size_type n = 1);

   size_type max_size() const noexcept;

   template <class U, class... Args> void construct(U* p, Args&&... args);
   template <class U> void destroy(U* p);

   template <class... Args> pointer new_element(Args&&... args);
   void delete_element(pointer p);

 private:
   union slot_t {
     value_type element;
     slot_t *next;
   };

   typedef char *data_pointer_t;
   typedef slot_t slot_type_t;
   typedef slot_t * slot_pointer_t;

   slot_pointer_t currentblock_;
   slot_pointer_t currentslot_;
   slot_pointer_t lastslot_;
   slot_pointer_t freeslot_;

   size_type pad_pointer(data_pointer_t p, size_type align) const noexcept;
   void allocate_block();

   static_assert(BlockSize >= 2 * sizeof(slot_type_t), "BlockSize too small.");
};

} // namespace memory

} // namespace pf_sys

#include "pf/sys/memory/pool.tcc"

#endif // PF_SYS_MEMORY_POOL_H_
