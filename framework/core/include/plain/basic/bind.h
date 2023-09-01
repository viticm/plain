/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id bind.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/07/10 16:00
 * @uses The bind template(tool for bind call functions).
 *       Refer: https://github.com/David-Haim/concurrencpp
 */

#ifndef PLAIN_BASIC_BIND_H_
#define PLAIN_BASIC_BIND_H_

#include "plain/basic/config.h"

namespace plain {

template <typename F>
auto &&bind(F &&function) {
  return std::forward<F>(function);
}

template <typename F, typename ...Args>
auto bind(F &&function, Args &&...args) {
  constexpr static auto inti = std::is_nothrow_invocable_v<F, Args...>;
  return [function = std::forward<F>(function),
         tuple = std::make_tuple(std::forward(args)...)]()
           mutable noexcept(inti) -> decltype(auto) {
    return std::apply(function, tuple);
  };
}

template <typename F>
auto &&bind_with_try_catch_impl(std::true_type, F &&function) {
  return std::forward<F>(function);
}

template <typename F>
auto &&bind_with_try_catch_impl(std::false_type, F &&function) {
  return [function = std::forward<F>(function)]() mutable noexcept {
    try {
      function();
    } catch(...) {
      // do nothing
    }
  };
}

template <typename F>
auto bind_with_try_catch(F &&function) {
  using is_noexcept = typename std::is_nothrow_invocable<F>::value;
  return bind_with_try_catch(is_noexcept{}, function);
}

template <typename F, typename ...Args>
auto bind_with_try_catch(F &&function, Args &&...args) {
  return bind_with_try_catch(
    bind(std::forward<F>(function), std::forward<Args>(args)...));
}

} // namespace plain

#endif // PLAIN_BASIC_BIND_H_
