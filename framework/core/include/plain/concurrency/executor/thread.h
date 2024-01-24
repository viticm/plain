/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id thread.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/09/05 17:43
 * @uses The concurrency thread executor implemention.
 */

#ifndef PLAIN_CONCURRENCY_EXECUTOR_THREAD_H_
#define PLAIN_CONCURRENCY_EXECUTOR_THREAD_H_

#include "plain/concurrency/executor/config.h"
#include <list>
#include <span>
#include <mutex>
#include <condition_variable>
#include "plain/sys/thread.h"
#include "plain/concurrency/executor/derivable.h"

namespace plain::concurrency {
namespace executor {

class PLAIN_API alignas(kCacheInlineAlignment)
Thread final : public Derivable<Thread> {

 public:
  Thread(
    const std::function<void(std::string_view name)> &started_callback = {},
    const std::function<void(std::string_view name)> &terminated_callback = {});
  ~Thread();

 public:
  void enqueue(Task task) override;
  void enqueue(std::span<Task> tasks) override;
  int32_t max_concurrency_level() const noexcept override;

  bool shutdown_requested() const override;
  void shutdown() override;

 private:
  std::mutex lock_;
  std::list<thread_t> workers_;
  std::condition_variable condition_;
  std::list<thread_t> last_retired_;
  bool abort_{false};
  std::atomic_bool atomic_abort_{false};
  const std::function<void(std::string_view name)> started_callback_;
  const std::function<void(std::string_view name)> terminated_callback_;

 private:
  void enqueue_impl(std::unique_lock<std::mutex> &lock, Task &task);
  void retire_worker(std::list<thread_t>::iterator it);

};

} // namespace executor
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_EXECUTOR_THREAD_H_
