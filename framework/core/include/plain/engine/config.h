/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id config.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/04/01 19:48
 * @uses The engine config file.
 */

#ifndef PLAIN_ENGINE_CONFIG_H_
#define PLAIN_ENGINE_CONFIG_H_

#include "plain/basic/config.h"

namespace plain {

enum class AppStatus {
  Running = 0,
  Stopped,
};

using console_func = 
  std::function<std::string(const std::vector<std::string> &args)>;

class Kernel;
class TimerQueue;
class Timer;

struct PLAIN_API engine_option {
  size_t max_cpu_threads;
  std::chrono::milliseconds max_thread_pool_executor_waiting_time;

  size_t max_background_threads;
  std::chrono::milliseconds max_background_executor_waiting_time;

  std::chrono::milliseconds max_timer_queue_waiting_time;

  std::function<void(std::string_view thread_name)> thread_started_callback;
  std::function<void(std::string_view thread_name)> thread_terminated_callback;

  engine_option() noexcept;
  engine_option(const engine_option &) = default;
  engine_option &operator=(const engine_option &) = default;
};

} // namespace plain

#endif // PLAIN_ENGINE_CONFIG_H_
