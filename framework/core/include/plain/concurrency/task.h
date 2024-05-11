/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id task.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/08/31 18:31
 * @uses The concurrency task implemention.
 */

#ifndef PLAIN_CONCURRENCY_TASK_H_
#define PLAIN_CONCURRENCY_TASK_H_

#include "plain/concurrency/config.h"
#include <utility>
#include <cassert>
#include "plain/basic/noncopyable.h"

namespace plain::concurrency {

namespace detail {

struct task_constants {
  static constexpr size_t total_size = 64;
  static constexpr size_t buffer_size = total_size - sizeof(void *);
};

struct vtable {
  void (*move_destroy_fn)(void *src, void *dst) noexcept;
  void (*execute_destroy_fn)(void *target);
  void (*destroy_fn)(void *target) noexcept;

  vtable(const vtable &) noexcept = default;
  constexpr vtable() noexcept : move_destroy_fn{nullptr},
    execute_destroy_fn{nullptr}, destroy_fn{nullptr} {}
  constexpr vtable(decltype(move_destroy_fn) _move_destroy_fn,
                   decltype(execute_destroy_fn) _execute_destroy_fn,
                   decltype(destroy_fn) _destroy_fn) noexcept :
    move_destroy_fn(_move_destroy_fn),
    execute_destroy_fn(_execute_destroy_fn), destroy_fn(_destroy_fn) {}
  static constexpr bool trivially_copiable_destructible(
    decltype(move_destroy_fn) move_fn) noexcept {
    return move_fn == nullptr;
  }
  static constexpr bool trivially_destructible(
    decltype(destroy_fn) destroy_fn) noexcept {
    return destroy_fn == nullptr;
  }
};

template <typename T>
class CallableVtable {

 public:
  static constexpr bool is_inlinable() noexcept {
    return std::is_nothrow_move_constructible_v<T> &&
      sizeof(T) <= task_constants::buffer_size;
  }

  template <typename CT>
  static void build(void *dst, CT &&callable) {
    if (is_inlinable()) {
      return build_inlinable(dst, std::forward<CT>(callable));
    }
    build_allocated(dst, std::forward<CT>(callable));
  }

  static void move_destroy(void *src, void *dst) noexcept {
    assert(src != nullptr);
    assert(dst != nullptr);
    if (is_inlinable())
      return move_destroy_inline(src, dst);
    return move_destroy_allocated(src, dst);
  }

  static void execute_destroy(void *target) {
    assert(target != nullptr);

    if (is_inlinable())
      return execute_destroy_inline(target);
    return execute_destroy_allocated(target);
  }

  static void destroy(void *target) noexcept {
    assert(target != nullptr);
    if (is_inlinable())
      return destroy_inline(target);
    return destroy_allocated(target);
  }

  static constexpr T *as(void *src) noexcept {
    if (is_inlinable())
      return inline_ptr(src);
    return allocated_ptr(src);
  }

 private:
  static T *inline_ptr(void *src) noexcept {
    return static_cast<T *>(src);
  }

  static T *allocated_ptr(void *src) noexcept {
    return *static_cast<T **>(src);
  }

  static T *&allocated_ref_ptr(void *src) noexcept {
    return *static_cast<T **>(src);
  }

  static void move_destroy_inline(void *src, void *dst) noexcept {
    auto callable_ptr = inline_ptr(src);
    new (dst) T(std::move(*callable_ptr));
    callable_ptr->~T();
  }

  static void move_destroy_allocated(void *src, void *dst) noexcept {
    auto callable_ptr = std::exchange(allocated_ref_ptr(src), nullptr);
    new (dst) T*(callable_ptr);
  }

  static void execute_destroy_inline(void *target) {
    auto callable_ptr = inline_ptr(target);
    (*callable_ptr)();
    callable_ptr->~T();
  }

  static void execute_destroy_allocated(void *target) {
    auto callable_ptr = allocated_ptr(target);
    (*callable_ptr)();
    delete callable_ptr;
  }

  static void destroy_inline(void *target) noexcept {
    auto callable_ptr = inline_ptr(target);
    callable_ptr->~T();
  }

  static void destroy_allocated(void *target) noexcept {
    auto callable_ptr = allocated_ptr(target);
    delete callable_ptr;
  }

  static constexpr vtable make_vtable() noexcept {
    void (*move_destroy_fn)(void *src, void *dst) noexcept = nullptr;
    void (*destroy_fn)(void *target) noexcept = nullptr;

    if constexpr (std::is_trivially_copy_constructible_v<T> && 
        std::is_trivially_destructible_v<T> && is_inlinable()) {
      move_destroy_fn = nullptr;
    } else {
      move_destroy_fn = move_destroy;
    }
    if constexpr (std::is_trivially_destructible_v<T> && is_inlinable()) {
      destroy_fn = nullptr;
    } else {
      destroy_fn = destroy;
    }
    return vtable(move_destroy_fn, execute_destroy, destroy_fn);
  }

  template <typename CT>
  static void build_inlinable(void *dst, CT &&callable) {
    new (dst) T(std::forward<CT>(callable));
  }

  template <typename CT>
  static void build_allocated(void *dst, CT &&callable) {
    auto new_ptr = new T(std::forward<CT>(callable));
    new (dst) T*(new_ptr);
  }

 public:
  static constexpr inline vtable vtable_ = CallableVtable::make_vtable();

};

} // namespace detail

class PLAIN_API Task {

 public:
  Task() noexcept;
  Task(Task &&rhs) noexcept;
  Task(coroutine_handle<void> handle) noexcept;

  template <typename T>
  Task(T &&callable) {
    build(std::forward<T>(callable));
  }

  ~Task() noexcept;

  Task &operator=(Task &&rhs) noexcept;

 public:
  void operator()();
  explicit operator bool() const noexcept;

 public:
  void clear() noexcept;
  template <typename T>
  bool contains() const noexcept {
    using decayed_type = typename std::decay_t<T>;
    if constexpr (std::is_same_v<decayed_type, coroutine_handle<void>>) {
      return contains_coroutine_handle();
    }
    return vtable_ == &detail::CallableVtable<decayed_type>::vtable_;
  }

 private:
  void build(Task &&rhs) noexcept;
  void build(coroutine_handle<void> handle) noexcept;

  template <typename T>
  void build(T &&callable) {
    using decayed_type = typename std::decay_t<T>;
    detail::CallableVtable<decayed_type>::build(
      buffer_, std::forward<T>(callable));
    vtable_ = &detail::CallableVtable<decayed_type>::vtable_;
  }

  template <typename T>
  static bool contains(const detail::vtable *const vtable) noexcept {
    return vtable == &detail::CallableVtable<T>::vtable_;
  }

  bool contains_coroutine_handle() const noexcept;

 private:
  alignas(std::max_align_t)
    std::byte buffer_[detail::task_constants::buffer_size];
  const detail::vtable *vtable_;

};

} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_TASK_H_
