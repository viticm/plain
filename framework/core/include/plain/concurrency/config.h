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

namespace executor {

class Basic;
class Inline;
class ThreadPool;
class Thread;
class WorkerThread;
class Manual;

} // namespace executor

struct executor_tag {

};

class Task;

template <typename T>
concept is_result_type = 
  std::is_same_v<T, void> || std::is_nothrow_move_constructible_v<T>;

template <typename T>
concept is_result_generator_type = !std::is_same_v<T, void>;

template <typename T>
concept is_base_of_executor = std::is_base_of_v<executor::Basic, T>;

template <typename T>
using coroutine_handle = std::coroutine_handle<T>;
using suspend_never = std::suspend_never;
using suspend_always = std::suspend_always;

template <typename T>
class Generator;

template <typename T>
class LazyResult;

template <typename T>
requires is_result_type<T>
class Result;

enum class ResultStatus : int32_t {
  Idle,
  Value,
  Exception,
};

namespace result {

template <typename T>
class Shared;

template <typename T>
struct WhenAny;

struct null {};

} // namespace result

} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_CONFIG_H_
