/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id config.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/07/16 11:00
 * @uses The concurrency result detail(implemention).
 */

#ifndef PLAIN_CONCURRENCY_RESULT_DETAIL_CONFIG_H_
#define PLAIN_CONCURRENCY_RESULT_DETAIL_CONFIG_H_

#include "plain/concurrency/result/config.h"

namespace plain::concurrency {
namespace result::detail {

// The result process status.
enum class ProcessStatus : int32_t {
  Idle,
  ConsumerSet,
  ConsumerWaiting,
  ConsumerDone,
  ProducerDone,
};

// The consumer status.
enum class ConsumerStatus : int32_t {
  Idle,
  Await,
  WhenAny,
  Shared
};

class StateBasic;

template <typename T>
class State;

class WhenAnyContext;

class SharedStateBasic;

template <typename T>
class SharedState;

template <typename T>
class LazyState;

template <typename T>
class ProducerContext;

class ConsumerContext;

class when_helper;
struct shared_helper;

} // namespace result::detail
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_RESULT_DETAIL_CONFIG_H_
