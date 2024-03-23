/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id pool.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/04/06 21:39
 * @uses The memory pool with thread safe.
 *       @refer: https://github.com/DimaBond174/FastMemPool
 */

#ifndef PLAIN_SYS_MEMORY_FAST_POOL_H_
#define PLAIN_SYS_MEMORY_FAST_POOL_H_

#include "plain/sys/memory/config.h"

#ifndef PLAIN_FP_LEAF_SIZE_BYTES
#define PLAIN_FP_LEAF_SIZE_BYTES  65535
#endif

#ifndef PLAIN_FP_LEAF_COUNT
#define PLAIN_FP_LEAF_COUNT  16
#endif

#ifndef PLAIN_FP_AVERAGE_ALLOCATION
#define PLAIN_FP_AVERAGE_ALLOCATION  655
#endif

#ifndef PLAIN_FP_RAISZE_EXEPTIONS
#define PLAIN_FP_RAISZE_EXEPTIONS  true
#endif

#ifndef PLAIN_FP_DO_OS_MALLOC
#define PLAIN_FP_DO_OS_MALLOC  true
#endif

namespace plain::memory {

template <
  int32_t kLeafSizeBytes = PLAIN_FP_LEAF_SIZE_BYTES,
  int32_t kLeafCount = PLAIN_FP_LEAF_COUNT,
  int32_t kAverageAllocation = PLAIN_FP_AVERAGE_ALLOCATION,
  bool kDoOSMalloc = PLAIN_FP_DO_OS_MALLOC,
  bool kRaiseExeptions = PLAIN_FP_RAISZE_EXEPTIONS>
class Pool {
public:
  /**
   * @brief malloc
   * Allocation function instead of malloc
   * @param allocation_size  -  volume to allocate
   * @return - allocation ptr
   */
  void* malloc(std::size_t allocation_size) {
    // Allocation will include a header with service information:
    const int32_t real_size = static_cast<int32_t>(
        allocation_size + sizeof(AllocHeader));
    // Starting leaf for finding the allocation place:
    const int32_t start_leaf = static_cast<int32_t>(
        cur_leaf_.load(std::memory_order_relaxed));
    // Selected leaf identifier:
    int32_t leaf_id = start_leaf;
    // Resulting allocation:
    char *re = nullptr;

    /*
      Exit the loop at the end of the loop when we meet start_leaf again.
      If it is impossible to make an allocation in your own memory pool,
      an escalation to OS malloc will occur, but the access control
      functionality will remain operational.
    */
    do {
      const int32_t available =
        leaf_array_[leaf_id].available.load(std::memory_order_acquire);
      // std::cout << "available: " << available << std::endl;
      if (available <= 0) break;
      if (available >= real_size) {
        // we reserve memory 
        // (the buffer is distributed from the end with a bite):
        const int32_t available_after =
          leaf_array_[leaf_id].available.fetch_sub(
              real_size, std::memory_order_acq_rel) - real_size;
        // and if successful, a positive number should have returned,
        // otherwise we would have broken through the bottom of the buffer:
        if (available_after >= 0) {
          // the resulting distribution address is easy to obtain, because it
          // starts immediately after "available", since addressing from &[0]
          // then this is "buf + available":
          re = leaf_array_[leaf_id].buf + available_after;
          if (available_after < kAverageAllocation) {
            // Let's tell the rest of the threads to use a different
            // memory page:
            const int32_t next_id = start_leaf + 1;
            if (next_id >= kLeafCount) {
              cur_leaf_.store(0, std::memory_order_release);
            } else {
              cur_leaf_.store(next_id, std::memory_order_release);
            }
          }
          break; // finished the search, return the allocation pointer
        }
      }
      ++leaf_id;
      if (kLeafCount == leaf_id) { leaf_id = 0; }
    } while (leaf_id != start_leaf);

    bool do_os_malloc = !re;
    if (do_os_malloc) { // Now the escalation to OS malloc will occur:
      if (kDoOSMalloc) {
        re = static_cast<char*>(::malloc(real_size));
#if defined(PLAIN_FP_AUTO_DEALLOCATE)
   #if not defined(Debug)
        if (re) {
          std::lock_guard<std::mutex> lg(mut_set_alloc_info_);
          set_alloc_info_.emplace(re);
        }
  #endif
#endif
      } else {
        if (kRaiseExeptions) {
          throw std::range_error(
              "Pool::malloc: need do_os_malloc, but it disabled");
        }
      }
    }

    if (re) { // if the allocation was successful, then fill in the header:
      AllocHeader *head = reinterpret_cast<AllocHeader *>(re);
      if (do_os_malloc) {
        head->leaf_id = kOSMallocID;
        head->tag_this = kTagOSMalloc;
      } else {
        head->leaf_id = leaf_id;
        head->tag_this = ((uint64_t)this) + leaf_id;
      }
      head->size = (int32_t)allocation_size;
      return (re + sizeof(AllocHeader));
    }

    return  nullptr;
  }  // malloc

  /**
   * @brief free  -  function to release allocation instead of "free"
   * @param ptr  -  allocation pointer obtained earlier via fmaloc
   */
  void free(void* ptr) {
    // Rewind back to get the AllocHeader:
    char *to_free = static_cast<char *>(ptr) - sizeof(AllocHeader);
    AllocHeader *head = reinterpret_cast<AllocHeader *>(to_free);
    if (0 <= head->size && head->size < kLeafSizeBytes
        && 0 <= head->leaf_id && head->leaf_id < kLeafCount
        && ((uint64_t)this) == (head->tag_this - head->leaf_id)
        && leaf_array_[head->leaf_id].buf) {
      // ok this is my allocation
      const int32_t real_size = head->size + sizeof(AllocHeader);
      const int32_t deallocated =
        leaf_array_[head->leaf_id].deallocated.fetch_add(
            real_size, std::memory_order_acq_rel) + real_size;
      int32_t available =
        leaf_array_[head->leaf_id].available.load(std::memory_order_acquire);
      if (deallocated == (kLeafSizeBytes - available)) {
        // everything that was allocated is now returned, we will try, carefully, reset the Leaf
        if (leaf_array_[head->leaf_id].available.compare_exchange_strong(
              available, kLeafSizeBytes)) {
          leaf_array_[head->leaf_id].deallocated -= deallocated;
        }
      }

      // Cleanup so that unique TAG_my_alloc will be keep unique in RAM:
      memset((char *)head, 0, sizeof(AllocHeader));
    } else if ((uint64_t)kTagOSMalloc == head->tag_this
        && kOSMallocID == head->leaf_id
        && head->size > 0) {
      // ok, to OS malloc
      // Cleanup so that unique TAG_my_alloc will be keep unique in RAM:
      memset((char *)head,  0,  sizeof(AllocHeader));
#if defined(PLAIN_FP_AUTO_DEALLOCATE)
   #if !_DEBUG
        {
          std::lock_guard<std::mutex> lg(mut_set_alloc_info_);
          set_alloc_info_.erase(head);
        }
  #endif
#endif
      ::free(head);
    } else {
      // this is someone else's allocation, Exception
      if (kRaiseExeptions) {
        throw std::range_error(
            "Pool::free: this is someone else's allocation");
      }
    }
    return;
  }

  /**
   * @brief check_access - checking the accessibility of the target memory area
   * @param base_alloc_ptr - the assumed address of the base
   * allocation from Pool
   * @param target_ptr - start of target memory area
   * @param target_size - the size of the structure to access
   * @return - true if the target memory area belongs to the base
   * allocation from Pool
   * Advantages: allows you to determine whether you have climbed
   * into another OWN allocation,
   * while the OS swears only if it climbed out of process RAM
   */
  bool check_access(
      void* base_alloc_ptr, void* target_ptr, std::size_t target_size) {
    bool re = false;
    AllocHeader *head = reinterpret_cast<AllocHeader *>(
        static_cast<char *>(base_alloc_ptr)  -  sizeof(AllocHeader));
    if (0 <= head->size && head->size < kLeafSizeBytes
         && 0 <= head->leaf_id && head->leaf_id < kLeafCount
         && ((uint64_t)this) == (head->tag_this - head->leaf_id)) {
      // ok, this is Pool allocation
      char *start = static_cast<char *>(base_alloc_ptr);
      char *end = start  +  head->size;
      char *buf = leaf_array_[head->leaf_id].buf;
      if (buf && buf <= start && (buf + kLeafSizeBytes) >= end){
        // Let's check whether it has gone beyond the allocation limits:
        char *target_start = static_cast<char *>(target_ptr);
        char *target_end = target_start + target_size;
        if (start <= target_start && target_end <= end) {
          re = true;
        }  else  {
          if (kRaiseExeptions) {
            throw std::range_error("Pool::check_access: out of allocation");
          }
        } // elseif (start  >  target_start
      }  else {
        if (kRaiseExeptions) {
          throw std::range_error("Pool::check_access: out of Leaf");
        }
      } // elseif (!buf
    } else if (kTagOSMalloc == head->tag_this
         && kOSMallocID == head->leaf_id
         && head->size > 0) {
      // Let's check whether it has gone beyond the allocation limits:
      char *start = static_cast<char *>(base_alloc_ptr);
      char *end = start + head->size;
      char *target_start = static_cast<char *>(target_ptr);
      char *target_end = target_start + target_size;
      if  (start <= target_start && target_end <= end) {
        re = true;
      }  else  {
        if (kRaiseExeptions) {
          throw std::range_error(
              "Pool::check_access: out of OS malloc allocation");
        }
      } // elseif  (start  >  target_start

    } else {
      if (kRaiseExeptions) {
        throw std::range_error(
            "Pool::check_access: not  Pool's allocation");
      }
    }
    return  re;
  } // check_access

  /**
   * @brief Pool - construct
   */
  Pool() noexcept {
    void* buf_array[kLeafCount];
    for (int32_t i = 0; i < kLeafCount; ++i) {
      buf_array[i] = ::malloc(kLeafSizeBytes);
    }
    std::sort(
        std::begin(buf_array),
        std::end(buf_array),
        [](const void* lh, const void* rh) {
          return (uint64_t)(lh) < (uint64_t)(rh);});
    // uint64_t last = 0;
    for (int32_t i = 0; i < kLeafCount; ++i) {
      if (buf_array[i]) {
        leaf_array_[i].buf = static_cast<char *>(buf_array[i]);
        leaf_array_[i].available.store(
            kLeafSizeBytes, std::memory_order_relaxed);
        leaf_array_[i].deallocated.store(0, std::memory_order_relaxed);
      } else {
        leaf_array_[i].buf = nullptr;
        leaf_array_[i].available.store(0, std::memory_order_relaxed);
        leaf_array_[i].deallocated.store(
            kLeafSizeBytes, std::memory_order_relaxed);
      }
    }
    std::atomic_thread_fence(std::memory_order_release);
    return;
  }  // Pool

  /**
   * @brief instance - Singleton implementation,
   * if anyone needs it all of a sudden
   * @return
    IMPORTANT: we rely on the linker to be able to remove duplicates
    of this method from all
    translation units .. provided the template parameters are the same
   */
  static Pool* instance() {
    static Pool obj;
    return &obj;
  }

  ~Pool() {
    for (int32_t i = 0; i < kLeafCount; ++i) {
      if (leaf_array_[i].buf) ::free(leaf_array_[i].buf);
    }
#if defined(PLAIN_FP_AUTO_DEALLOCATE)
    #if _DEBUG
    {
      std::lock_guard<std::mutex> lg(mut_map_alloc_info_);
      for (auto &&it : map_alloc_info_) {
        ::free(it.first);
      }
      map_alloc_info_.clear();
    }
    #else
    {
      std::lock_guard<std::mutex>  lg(mut_set_alloc_info_);
      for (auto &&it : set_alloc_info_) {
        //std::cout << " auto deallocated :" << (uint64_t(it)) << std::endl;
        ::free(it);
      }
      set_alloc_info_.clear();
    }
    #endif
#endif
    return;
  }

  Pool& operator = (const Pool& ) = delete;
  Pool (const Pool& ) = delete;

  Pool& operator = (Pool&&) = delete;
  Pool (Pool&&) = delete;

#if _DEBUG
  /**
   * @brief mallocd
   * Decorator for malloc method - stores information about the 
   * location of the allocation
   * @param filename
   * @param line
   * @param function_name
   * @param allocation_size
   * @return
   */
  void* mallocd(const char *filename,
                uint32_t line,
                const char *function_name,
                std::size_t allocation_size) {
    void* re = malloc(allocation_size);
    if (re) {
      std::lock_guard<std::mutex>  lg(mut_map_alloc_info_);
      auto it = map_alloc_info_.try_emplace(re,  AllocInfo());
      if (it.first->second.allocated)  {
        std::string err("Pool::mallocd: already allocated by ");
        err.append(it.first->second.who);
        throw std::range_error(err);
      } else {
        auto &&who = it.first->second.who;
        who.clear();
        who.append(filename).append(", at ")
            .append(std::to_string(line)).append("  line, in ").append(function_name);
        it.first->second.allocated = true;
      }
    }
    return re;
  }

  /**
   * @brief freed
   * Decorator for the free method
   * - saves information about the place of deallocation
   * - in case of re-deallocation, tells where the first deallocation took place
   * @param filename
   * @param line
   * @param function_name
   * @param ptr
   */
  void freed(const char *filename,
             uint32_t line,
             const char *function_name,
             void* ptr) {
    if (ptr) {
      {
        std::lock_guard<std::mutex> lg(mut_map_alloc_info_);
        auto it = map_alloc_info_.find(ptr);
        if (map_alloc_info_.end() == it) {
          throw std::range_error(
              "Pool::freed: this pointer has never been allocated");
        }
        if (!it->second.allocated) {
          std::string err(
              "Pool::freed: this pointer has already been freed from: ");
          err.append(it->second.who);
          throw std::range_error(err);
        }
        auto &&who = it->second.who;
        who.clear();
        who.append(filename).append(", at ")
            .append(std::to_string(line))
            .append("  line, in ").append(function_name);
        it->second.allocated = false;
      }
      ::free(ptr);
    }
    return;
  } // freed

  bool check_accessd(const char *filename,
                     uint32_t line,
                     const char *function_name,
                     void* base_alloc_ptr,
                     void* target_ptr,
                     std::size_t target_size) {
    bool re = false;
    try {
      re = check_access(base_alloc_ptr, target_ptr, target_size);
    } catch (std::range_error e) {

    }

    if (!re) {
      std::string who("Pool::check_accessd buffer overflow at ");
      who.append(filename).append(", at ")
          .append(std::to_string(line)).append("  line, in ").append(function_name);
      if (kRaiseExeptions) {
        throw std::range_error(who);
      }
    }
    return  re;
  } // check_accessd

#endif
private:
  struct leaf_t {
    char *buf;
    // available == offset
    std::atomic<int32_t> available{ kLeafSizeBytes };
    // control of deallocations:
    std::atomic<int32_t> deallocated{ 0 };
  };

  /*
   * AllocHeader
    Allows you to quickly assess whether your allocation and where:
    tag_this - a unique number unlikely to occur in RAM,
       while there is a mutual check with leaf_id:
      if (tag_this - leaf_id)  !=  this  and  OS malloc != -kOSMallocID,
      then this is someone else's allocation
    leaf_id == {  -2020071708  } => allocation was done in OS malloc,
    leaf_id >= 0 => allocation in leaf_array_[leaf_id],
    size - allocation size, if 0 <= size> = LeafSizeBytes - it means someone
    else's allocation.
    The header can be obtained at any time by a negative offset relative to
    the *pointer.
  */
  const int32_t kOSMallocID{ -2020071708 };
  const int32_t kTagOSMalloc{ 1020071708 };
  struct AllocHeader {
    /*
     label of own allocations:
     tag_this = (uint64_t)this + leaf_id */
    uint64_t tag_this{ 2020071700 };
    // allocation size (without sizeof(AllocHeader)):
    int32_t size;
    // allocation place id (Leaf ID  or kOSMallocID):
    int32_t leaf_id{ -2020071708 };
  };

  // Memory pool:
  leaf_t leaf_array_[kLeafCount];
  std::atomic<int32_t> cur_leaf_{ 0 };

#if _DEBUG
  struct AllocInfo {
    std::string who;  // who performed the operation: file, code line number,
                      // in which method
    bool allocated{ false };  // true - allocated, false - deallocated
  };
  std::map<void* , AllocInfo> map_alloc_info_;
  std::mutex mut_map_alloc_info_;

  // Just for easy viewing in debug:
  int32_t leaf_size_bytes_{ kLeafSizeBytes };
  int32_t leaf_count_{ kLeafCount };
  int32_t average_allocation_{ kAverageAllocation };
  bool do_os_malloc_{ kDoOSMalloc };
  bool raise_exeptions_{ kRaiseExeptions };
#else
  #if defined(PLAIN_FP_AUTO_DEALLOCATE)
  std::set<void* > set_alloc_info_;
  std::mutex mut_set_alloc_info_;
  #endif
#endif
};

/**
   * @brief PLAIN_MALLOC
   * Allocation function instead of malloc
   * @param fast_pool  -  an instance of Pool in which we allocate
   * @param allocation_size  -  volume to allocate
   * @return - allocation ptr
*/
#if _DEBUG
#define PLAIN_MALLOC(fast_pool, allocation_size) \
   (fast_pool)->mallocd(__FILE__, __LINE__, __FUNCTION__, allocation_size)
#else
#define PLAIN_MALLOC(fast_pool, allocation_size) \
   (fast_pool)->malloc(allocation_size)
#endif

/**
 * @brief PLAIN_FREE  -  function to release allocation instead of "free"
 * @param fast_pool  - an instance of Pool in which we allocate
 * @param ptr  -  allocation pointer obtained earlier via fmaloc
 */
#if _DEBUG
#define PLAIN_FREE(fast_pool, ptr) \
   (fast_pool)->freed(__FILE__, __LINE__, __FUNCTION__, ptr)
#else
#define PLAIN_FREE(fast_pool, ptr) \
   (fast_pool)->free(ptr)
#endif

/**
   * @brief PLAIN_CHECK_ACCESS  -  checking the accessibility of the
   * target memory area
   * @param fast_pool - an instance of Pool in which we allocate
   * @param base_alloc_ptr - the assumed address of the base allocation
   * from Pool
   * @param target_ptr - start of target memory area
   * @param target_size - the size of the structure to access
   * @return - true if the target memory area belongs to the base
   * allocation from Pool
   * Advantages: allows you to determine whether you have climbed into
   * another OWN allocation,
   * while the OS swears only if it climbed out of process RAM
 */
#if _DEBUG
#define PLAIN_CHECK_ACCESS(fast_pool, base_alloc_ptr, target_ptr, target_size) \
   (fast_pool)->check_accessd(__FILE__, __LINE__, __FUNCTION__, \
       base_alloc_ptr, target_ptr, target_size)
#else
#define PLAIN_CHECK_ACCESS(fast_pool, base_alloc_ptr, target_ptr, target_size) \
   (fast_pool)->check_access(base_alloc_ptr, target_ptr, target_size)
#endif

struct PoolNull {
  // Null object
};

/**
 * PoolAllocator
 * == std::allocator<T> template
 * Default works with SingleTone Pool<>::instance()
 * Pool can be injected as template method or allocation strategy:

 // inject template (Template method):
 PoolAllocator<std::string, Pool<111, 11> > myAllocator;

 // inject instance (Strategy):
  using MyAllocatorType = Pool<333, 33>;
  MyAllocatorType  fastMemPool;  // instance of
  PoolAllocator<std::string, MyAllocatorType > myAllocator(&fastMemPool);
 */
template<class T, class FAllocator = PoolNull >
struct PoolAllocator : public std::allocator<T>  {
  typedef T value_type;
  using MyAllocatorType = FAllocator;
  MyAllocatorType *allocator_{nullptr};
  PoolAllocator() = default;
  PoolAllocator(MyAllocatorType *in_allocator) : allocator_(in_allocator) {

  }
  template <class U> constexpr PoolAllocator(const PoolAllocator<U> &)
  noexcept {}

  T *allocate(std::size_t n) {
    if (n > std::numeric_limits<std::size_t>::max() / sizeof (T))
      throw std::bad_alloc();
    if (std::is_same<FAllocator, PoolNull>::value) {
      if (auto p = static_cast<T *>(
            PLAIN_MALLOC(Pool<>::instance(), (n * sizeof (T)))))
        return p;
    }  else {
      if (allocator_) {
        if (auto p = static_cast<T *>(PLAIN_MALLOC(allocator_, (n * sizeof (T)))))
          return p;
      } else {
        if (auto p = static_cast<T *>(
              PLAIN_MALLOC(FAllocator::instance(), (n * sizeof (T)))))
        return p;
      }
    }
    throw std::bad_alloc();
  } // alloc

  void deallocate(T *p, std::size_t) noexcept {
    if (std::is_same<FAllocator, PoolNull>::value) {
      PLAIN_FREE(Pool<>::instance(),  p);
    } else {
      if (allocator_) {
        PLAIN_FREE(allocator_,  p);
      } else {
        PLAIN_FREE(FAllocator::instance(),  p);
      }
    }
    return;
  }

  template <typename _Up, typename... _Args>
  void construct(_Up* __p, _Args &&... __args)
  { ::new((void* )__p) _Up(std::forward<_Args>(__args)...); }

  template<typename _Up>
  void destroy(_Up* __p) { __p->~_Up(); }
};

template <class T, class U>
bool operator == (const PoolAllocator<T>&, const PoolAllocator<U>&) {
  return true;
}

template<class T, class U>
bool operator != (const PoolAllocator<T>&, const PoolAllocator<U>&) {
  return false;
}

} // namespace plain::memory

#endif // PLAIN_SYS_MEMORY_FAST_POOL_H_
