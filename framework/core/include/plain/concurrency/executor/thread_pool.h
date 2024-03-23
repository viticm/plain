/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id thread_pool.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/09/06 14:41
 * @uses The concurrency thread pool executor implemention.
 */

#ifndef PLAIN_CONCURRENCY_EXECUTOR_THREAD_POOL_H_
#define PLAIN_CONCURRENCY_EXECUTOR_THREAD_POOL_H_

#include "plain/concurrency/executor/config.h"
#include "plain/concurrency/executor/derivable.h"

namespace plain::concurrency {
namespace executor {

namespace detail {

class IdleWorkerSet {

 public:
  IdleWorkerSet(size_t size);

 public:
  void set_idle(size_t idle_thread) noexcept;
  void set_active(size_t idle_thread) noexcept;

  size_t find_idle_worker(size_t caller_index) noexcept;
  void find_idle_workers(
    size_t caller_index, std::vector<size_t> &result_buffer,
    size_t max_count) noexcept;

 private:
  enum class Status {Active, Idle};
  struct alignas(kCacheInlineAlignment) padded_flag {
    std::atomic<Status> flag {Status::Active};
  };
 
 private:
  std::atomic_intptr_t approx_size_;
  const std::unique_ptr<padded_flag[]> idle_flags_;
  const size_t size_;

 private:
  bool try_acquire_flag(size_t index) noexcept;

};

class ThreadPoolWorker;

} // namespace detail


class PLAIN_API alignas(kCacheInlineAlignment)
ThreadPool final : public Derivable<ThreadPool> {

 public:
  ThreadPool(
    std::string_view pool_name,
    size_t pool_size, std::chrono::milliseconds max_idle_time,
    const std::function<void(std::string_view name)> &started_callback = {},
    const std::function<void(std::string_view name)> &terminated_callback = {});
  ~ThreadPool() override;

 public:
  void enqueue(Task task) override;
  void enqueue(std::span<Task> tasks) override;

  int32_t max_concurrency_level() const noexcept override;

  bool shutdown_requested() const override;
  void shutdown() override;

  std::chrono::milliseconds max_worker_idle_time() const noexcept;

 private:
  friend class detail::ThreadPoolWorker;

 private:
  std::vector<detail::ThreadPoolWorker> workers_;
  alignas(kCacheInlineAlignment) std::atomic_size_t round_robin_cursor_;
  alignas(kCacheInlineAlignment) detail::IdleWorkerSet idle_workers_;
  alignas(kCacheInlineAlignment) std::atomic_bool abort_;

 private:
  void mark_worker_idle(size_t index) noexcept;
  void mark_worker_active(size_t index) noexcept;
  void find_idle_workers(
    size_t caller_index, std::vector<size_t> &buffer, size_t max_count) noexcept;
  detail::ThreadPoolWorker &worker_at(size_t index) noexcept;

};

} // namespace executor
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_EXECUTOR_THREAD_POOL_H_
