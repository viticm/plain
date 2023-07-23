/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id config.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/07/16 10:38
 * @uses The concurrency config.
 */

#ifndef PLAIN_CONCURRENCY_CONFIG_H_
#define PLAIN_CONCURRENCY_CONFIG_H_

#include "plain/basic/config.h"
#include <coroutine>

namespace plain::concurrency {

template <typename T>
using coroutine_handle = std::coroutine_handle<T>;
using suspend_never = std::suspend_never;
using suspend_always = std::suspend_always;

template <typename T>
class Generator;

template <typename T>
class LazyResult;

template <typename T>
class Result;

enum class ResultStatus {
  Idle,
  Value,
  Exception,
};

} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_CONFIG_H_
