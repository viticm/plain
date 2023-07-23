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
enum class ProcessStatus {
  Idle,
  ConsumerSet,
  ConsumerWaiting,
  ConsumerDone,
  ProducerDone,
};

// The consumer status.
enum class ConsumerStatus {
  Idle,
  Await,
  WaitFor,
  WhenAny,
  Shared
};

class StateBasic;

template <typename T>
class State;

class SharedStateBasic;

template <typename T>
class SharedState;

template <typename T>
class LazyState;

template <typename T>
class ProducerContext;

struct WhenHelper;
struct SharedHelper;

} // namespace result::detail
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_RESULT_DETAIL_CONFIG_H_
