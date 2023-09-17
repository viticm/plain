/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id when.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/08/09 16:32
 * @uses The concurrency result when.
 */

#ifndef PLAIN_CONCURRENCY_RESULT_WHEN_H_
#define PLAIN_CONCURRENCY_RESULT_WHEN_H_

#include "plain/concurrency/result/config.h"
#include "plain/concurrency/result/resume_on.h"
#include "plain/concurrency/result/lazy.h"
#include "plain/concurrency/result/detail/state.h"

namespace plain::concurrency {
namespace result {

namespace detail {

class WhenHelper {

 public:
  template <typename T>
  static StateBasic &at(T &tuple, size_t n) noexcept {
    auto seq = std::make_index_sequence<std::tuple_size<T>::value>();
    return at_impl(seq, tuple, n);
  }

  template <typename T>
  static StateBasic &at(std::vector<Result<T>> &list, size_t n) noexcept {
    assert(n < list.size());
    return get_state_basic(list[n]);
  }

  template <typename ...T>
  static size_t size(std::tuple<T...> &tuple) noexcept {
      return std::tuple_size_v<std::tuple<T...>>;
  }

  template <typename T>
  static size_t size(const std::vector<T> &list) noexcept {
    return list.size();
  }

  template <typename ...R>
  static void assert_if_empty_tuple(const char* err, R &&...results) {
    assert_if_empty_impl(err, std::forward<R>(results)...);
  }

  template <typename Iter>
  static void assert_if_empty_range(const char* err, Iter begin, Iter end) {
    for (; begin != end; ++begin) assert(static_cast<bool>(*begin));
  }

 private:
  template <typename T>
  static StateBasic &get_state_basic(Result<T> &result) noexcept {
    assert(result.state_);
    return *result.state_;
  }

  template <typename T, size_t ...N>
  static StateBasic &
  at_impl(std::index_sequence<N...>, T &tuple, size_t n) noexcept {
    StateBasic *list[] = {&get_state_basic(std::get<N>(tuple))...};
    assert(list[n] != nullptr);
    return *list[n];
  }

  template <typename T, typename ...R>
  static void assert_if_empty_impl(
    const char *err, const Result<T> &result, R &&...results) {
    assert(static_cast<bool>(result));
    assert_if_empty(err, std::forward<R>(results)...);
  }

  static void assert_if_empty_impl([[maybe_unused]]const char *err) noexcept {
  }

 public:
  class WhenAllAwaitable {

   public:
    WhenAllAwaitable(StateBasic &state) noexcept : state_{state} {}
  
   public:
    bool await_ready() const noexcept {
      return false;
    }

    bool await_suspend(coroutine_handle<void> handle) noexcept {
      return state_.await(handle);
    }

    void await_resume() const noexcept {}

   private:
    StateBasic &state_;
  };

 public:
  template <typename T>
  class WhenAnyAwaitable {

   public:
    WhenAnyAwaitable(T &results) noexcept : results_{results} {}
    
    bool await_ready() const noexcept {
      return false;
    }

    bool await_suspend(coroutine_handle<void> handle) {
      promise_ = std::make_shared<WhenAnyContext>(handle);
      const auto range_length = WhenHelper::size(results_);
      for (size_t i = 0; i < range_length; i++) {
        if (promise_->any_result_finished()) return false;
        auto &state_ref = WhenHelper::at(results_, i);
        const auto status = state_ref.when_any(promise_);
        if (status == ProcessStatus::ProducerDone) {
          return promise_->resume_inline(state_ref);
        }
      }
      return promise_->finish_processing();
    }

    size_t await_resume() noexcept {
      const auto completed_result_state = promise_->completed_result();
      auto completed_result_index = std::numeric_limits<size_t>::max();
      const auto range_length = WhenHelper::size(results_);
      for (size_t i = 0; i < range_length; i++) {
        auto &state_ref = WhenHelper::at(results_, i);
        state_ref.try_rewind_consumer();
        if (completed_result_state == &state_ref) {
          completed_result_index = i;
          // FIXME: need break here?
        }
      }
      assert(completed_result_index != std::numeric_limits<size_t>::max());
      return completed_result_index;
    }
  
   private:
    std::shared_ptr<WhenAnyContext> promise_;
    T &results_;
  };

};

// FIXME: This micro define for assert.
#ifndef NDEBUG
#ifndef plian_concurrency_when_helper_assert_if_empty_tuple
#define plian_concurrency_when_helper_assert_if_empty_tuple \
  detail::WhenHelper::assert_if_empty_tuple
#endif
#ifndef plian_concurrency_when_helper_assert_if_empty_range
#define plian_concurrency_when_helper_assert_if_empty_range \
  detail::WhenHelper::assert_if_empty_range
#endif
#else
#define plian_concurrency_when_helper_assert_if_empty_tuple
#define plian_concurrency_when_helper_assert_if_empty_range
#endif

template <typename ET, typename CT>
LazyResult<CT> when_all_impl(
  std::shared_ptr<ET> resume_executor, CT collection) {
  for (size_t i = 0; i < WhenHelper::size(collection); i++) {
    auto &state_ref = WhenHelper::at(collection, i);
    co_await WhenHelper::WhenAllAwaitable{state_ref};
  }
  co_await resume_on(resume_executor);
  co_return std::move(collection);
}

template <typename E, typename T>
LazyResult<WhenAny<T>>
when_any_impl(std::shared_ptr<E> resume_executor, T tuple) {
  const auto completed_index = co_await WhenHelper::WhenAnyAwaitable<T>{tuple};
  co_await resume_on(resume_executor);
  co_return WhenAny<T>{completed_index, std::move(tuple)};
}

template <typename E, typename T>
LazyResult<WhenAny<std::vector<T>>>
when_any_impl(std::shared_ptr<E> resume_executor, std::vector<T> list) {
  const auto completed_index = co_await WhenHelper::WhenAnyAwaitable{list};
  co_await resume_on(resume_executor);
  co_return WhenAny<std::vector<T>>{completed_index, std::move(list)};
}

} // namespace detail

template <typename T>
struct WhenAny {
  size_t index;
  T results;

  WhenAny() noexcept : index{static_cast<size_t>(-1)} {}

  template <typename ...Args>
  WhenAny(size_t index, Args &&...results) noexcept
    : index(index), results(std::forward<Args>(results)...) {}

  WhenAny(WhenAny &&) noexcept = default;
  WhenAny &operator=(WhenAny &&) noexcept = default;

};

template <typename T>
LazyResult<std::tuple<>> when_all(std::shared_ptr<T> resume_executor) {
  if (!static_cast<bool>(resume_executor)) {
    return {};
  }
  auto make_lazy_result = []() -> LazyResult<std::tuple<>> {
    co_return std::tuple<>{};
  };
  return make_lazy_result();
}

template <typename T, typename ...R>
LazyResult<std::tuple<typename std::decay<R>::type...>>
when_all(std::shared_ptr<T> resume_executor, R &&...results) {
  assert(static_cast<bool>(resume_executor));
  return detail::when_all_impl(
    resume_executor, std::make_tuple(std::forward<R>(results)...));
}

template <typename E, typename Iter>
LazyResult<std::vector<typename std::iterator_traits<Iter>::value_type>>
when_all(std::shared_ptr<E> resume_executor, Iter begin, Iter end) {
  assert(static_cast<bool>(resume_executor));
  using type = typename std::iterator_traits<Iter>::value_type;
  return detail::when_all_impl(
    resume_executor,
    std::vector<type>{
      std::make_move_iterator(begin), std::make_move_iterator(end)});
}

template <typename E, typename ...R>
LazyResult<WhenAny<std::tuple<R ...>>>
when_any(std::shared_ptr<E> resume_executor, R &&...results) {
  static_assert(sizeof...(R) != 0,
    "the function must accept at least one result object.");
  plian_concurrency_when_helper_assert_if_empty_tuple(
    std::forward<R>(results)...);
  assert(static_cast<bool>(resume_executor));
  return detail::when_any_impl(
    resume_executor, std::make_tuple(std::forward<R>(results)...));
}

template <typename E, typename Iter>
LazyResult<
WhenAny<std::vector<typename std::iterator_traits<Iter>::value_type>>>
when_any(std::shared_ptr<E> resume_executor, Iter begin, Iter end) {
  plian_concurrency_when_helper_assert_if_empty_range(begin, end);
  assert(begin != end);
  assert(static_cast<bool>(resume_executor));
  using type = typename std::iterator_traits<Iter>::value_type;
  return detail::when_any_impl(resume_executor,
    std::vector<type>{
      std::make_move_iterator(begin), std::make_move_iterator(end)});
}

} // namespace result
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_RESULT_WHEN_H_
