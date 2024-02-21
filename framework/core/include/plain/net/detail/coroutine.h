/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id coroutine.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2024 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2024/02/06 17:12
 * @uses The detail for net implemention.
 */

#ifndef PLAIN_NET_DETAIL_COROUTINE_H_
#define PLAIN_NET_DETAIL_COROUTINE_H_

#include "plain/net/detail/config.h"
#include <cassert>
#include <optional>
#include <variant>
#include <utility>
#include "plain/concurrency/config.h"
#include "plain/net/connection/detail/config.h"

namespace plain::net {
namespace detail {

struct Resolver {
  virtual void resolve(int32_t result) noexcept = 0;
};

struct ResumeResolver final: Resolver {
  friend struct Awaitable;

  void resolve(int32_t _result) noexcept override {
    this->result = _result;
    handle.resume();
  }

 private:
  std::coroutine_handle<> handle;
  int32_t result{0};
};
static_assert(std::is_trivially_destructible_v<ResumeResolver>);

struct DeferredResolver final: Resolver {
  void resolve(int32_t _result) noexcept override {
    this->result = _result;
  }

#ifndef NDEBUG
  ~DeferredResolver() {
    assert(!!result && "DeferredResolver is destructed before it's resolved");
  }
#endif

  std::optional<int32_t> result;
};

struct CallbackResolver final: Resolver {
  CallbackResolver(
    std::function<void (int32_t result)>&& cb): cb_(std::move(cb)) {}

  void resolve(int32_t result) noexcept override {
    this->cb_(result);
    delete this;
  }

 private:
  std::function<void (int32_t result)> cb_;
};

struct Awaitable {
  // TODO: use cancel_token to implement cancellation
  Awaitable(std::function<void(void *)> set_data_func) noexcept:
    set_data_func_(set_data_func) {}

  bool set_data(void *data) {
    if (!static_cast<bool>(set_data_func_)) return false;
    set_data_func_(data);
    return true;
  }

  // User must keep Resolver alive before the operation is finished
  void set_deferred(DeferredResolver &resolver) {
    set_data(&resolver);
  }

  void set_callback(std::function<void (int32_t result)> cb) {
    set_data(new CallbackResolver(std::move(cb)));
  }

  auto operator co_await() {
    struct awaitable {
      ResumeResolver resolver{};
      std::function<void(void *)> set_data_func_;
      awaitable(std::function<void(void *)> set_data_func):
        set_data_func_(set_data_func) {}

      constexpr bool await_ready() const noexcept { return false; }

      void await_suspend(std::coroutine_handle<> handle) noexcept {
        resolver.handle = handle;
        if (static_cast<bool>(set_data_func_))
          set_data_func_(&resolver);
        else
          handle();
      }

      constexpr int32_t await_resume() const noexcept {
        return resolver.result;
      }
    };

    return awaitable(set_data_func_);
  }

 private:
  std::function<void(void *)> set_data_func_;
};


template <typename T, bool nothrow>
struct Task;

// only for internal usage
template <typename T, bool nothrow>
struct TaskPromiseBasic {
  Task<T, nothrow> get_return_object();
  auto initial_suspend() { return std::suspend_never(); }
  auto final_suspend() noexcept {
    struct Awaiter: std::suspend_always {
      TaskPromiseBasic *me_;

      Awaiter(TaskPromiseBasic *me): me_(me) {};
      std::coroutine_handle<>
      await_suspend(
        [[maybe_unused]] std::coroutine_handle<> caller) const noexcept {
        if (me_->result_.index() == 3) [[unlikely]] {
          // FIXME: destroy current coroutine; otherwise memory leaks.
          if (me_->waiter_) {
            me_->waiter_.destroy();
          }
          std::coroutine_handle<TaskPromiseBasic>::from_promise(*me_).destroy();
        } else if (me_->waiter_) {
          return me_->waiter_;
        }
        return std::noop_coroutine();
      }
    };
    return Awaiter(this);
  }
  void unhandled_exception() {
    if constexpr (!nothrow) {
      if (result_.index() == 3) [[unlikely]] return;
      result_.template emplace<2>(std::current_exception());
    } else {
#if OS_UNIX
      __builtin_unreachable();
#else
      std::terminate();
#endif
    }
  }

 protected:
  friend struct Task<T, nothrow>;
  TaskPromiseBasic() = default;
  std::coroutine_handle<> waiter_;
  std::variant<
    std::monostate,
    std::conditional_t<std::is_void_v<T>, std::monostate, T>,
    std::conditional_t<!nothrow, std::exception_ptr, std::monostate>,
    std::monostate // indicates that the promise is detached
  > result_;
};

// only for internal usage
template <typename T, bool nothrow>
struct TaskPromise final: TaskPromiseBasic<T, nothrow> {
  using TaskPromiseBasic<T, nothrow>::result_;

  template <typename U>
  void return_value(U&& u) {
    if (result_.index() == 3) [[unlikely]] return;
    result_.template emplace<1>(static_cast<U&&>(u));
  }
  void return_value(int u) {
    if (result_.index() == 3) [[unlikely]] return;
    result_.template emplace<1>(u);
  }
};

template <bool nothrow>
struct TaskPromise<void, nothrow> final: TaskPromiseBasic<void, nothrow> {
  using TaskPromiseBasic<void, nothrow>::result_;

  void return_void() {
    if (result_.index() == 3) [[unlikely]] return;
    result_.template emplace<1>(std::monostate {});
  }
};

/**
 * An awaitable object that returned by an async function
 * @tparam T value type holded by this task
 * @tparam nothrow if true, the coroutine assigned by this task won't
 *         throw exceptions (slightly better performance)
 * @warning do't discard this object when returned by some function,
 *         or ub will happen
 */
template <typename T = void, bool nothrow = false>
struct Task final {
  using promise_type = TaskPromise<T, nothrow>;
  using handle_t = std::coroutine_handle<promise_type>;

  Task(const Task &) = delete;
  Task &operator=(const Task &) = delete;

  bool await_ready() {
    auto& result_ = coro_.promise().result_;
    return result_.index() > 0;
  }

  template <typename _T, bool nothrow_>
  void await_suspend(
    std::coroutine_handle<TaskPromise<_T, nothrow_>> caller) noexcept {
    coro_.promise().waiter_ = caller;
  }

  T await_resume() const {
    return get_result();
  }

  /** Get the result hold by this task */
  T get_result() const {
    auto &result_ = coro_.promise().result_;
    assert(result_.index() != 0);
    if constexpr (!nothrow) {
      if (auto *pep = std::get_if<2>(&result_)) {
        std::rethrow_exception(*pep);
      }
    }
    if constexpr (!std::is_void_v<T>) {
      return *std::get_if<1>(&result_);
    }
  }

  /** Get is the coroutine done */
  bool done() const {
    return coro_.done();
  }

  /** Only for placeholder */
  Task(): coro_(nullptr) {}

  Task(Task &&other) noexcept {
    coro_ = std::exchange(other.coro_, nullptr);
  }

  Task &operator=(Task &&other) noexcept {
    if (coro_) coro_.destroy();
    coro_ = std::exchange(other.coro_, nullptr);
    return *this;
  }

  /** Destroy (when done) or detach (when not done) the task object */
  ~Task() {
    if (!coro_) return;
    if (!coro_.done()) {
      coro_.promise().result_.template emplace<3>(std::monostate{});
    } else {
      coro_.destroy();
    }
  }

private:
  friend struct TaskPromiseBasic<T, nothrow>;
  Task(promise_type *p): coro_(handle_t::from_promise(*p)) {}
  handle_t coro_;
};

template <typename T, bool nothrow>
Task<T, nothrow> TaskPromiseBasic<T, nothrow>::get_return_object() {
    return Task<T, nothrow>(static_cast<TaskPromise<T, nothrow> *>(this));
}

} // namespace detail
} // namespace plain::net

#endif // PLAIN_NET_DETAIL_COROUTINE_H_
