/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id worker_thread.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/09/11 18:28
 * @uses The concurrency worker thread executor implemention.
 */

#ifndef PLAIN_CONCURRENCY_EXECUTOR_WORKER_THREAD_H_
#define PLAIN_CONCURRENCY_EXECUTOR_WORKER_THREAD_H_

#include "plain/concurrency/executor/config.h"
#include <mutex>
#include <semaphore>
#include "plain/sys/thread.h"
#include "plain/concurrency/executor/derivable.h"

namespace plain::concurrency {
namespace executor {

class PLAIN_API alignas(kCacheInlineAlignment)
WorkerThread final : public Derivable<WorkerThread> {

 public:
  WorkerThread(
    const std::function<void(std::string_view)> &started_callback,
    const std::function<void(std::string_view)> &terminated_callback);

 public:
  void enqueue(Task task) override;
  void enqueue(std::span<Task> tasks) override;

  int32_t max_concurrency_level() const noexcept override;

  bool shutdown_requested() const override;
  void shutdown() override;

 private:
  std::deque<Task> private_queue_;
  std::atomic_bool private_atomic_abort_;
  alignas(kCacheInlineAlignment) std::mutex lock_;
  std::deque<Task> public_queue_;
  std::binary_semaphore semaphore_;
  thread_t thread_;
  std::atomic_bool atomic_abort_;
  bool abort_;
  const std::function<void(std::string_view)> started_callback_;
  const std::function<void(std::string_view)> terminated_callback_;

 private:
  void make_os_worker_thread();
  bool drain_queue_impl();
  bool drain_queue();
  void wait_for_task(std::unique_lock<std::mutex> &lock);
  void work_loop();

  void enqueue_local(Task &task);
  void enqueue_local(std::span<Task> tasks);

  void enqueue_foreign(Task &task);
  void enqueue_foreign(std::span<Task> tasks);

};

} // namespace executor
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_EXECUTOR_WORKER_THREAD_H_
