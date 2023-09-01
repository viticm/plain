#include "plain/concurrency/task.h"
#include "plain/concurrency/result/detail/consumer_context.h"
#include <cstring>

using plain::concurrency::Task;
using plain::concurrency::detail::vtable;
using plain::concurrency::detail::CallableVtable;
using plain::concurrency::result::detail::AwaitViaFunctor;

namespace plain::concurrency {

namespace {

class CoroutineHandleFunctor : noncopyable {

 public:
  CoroutineHandleFunctor() noexcept : coroutine_handle_{} {}
  CoroutineHandleFunctor(coroutine_handle<void> handle) noexcept :
    coroutine_handle_{handle} {}
  CoroutineHandleFunctor(CoroutineHandleFunctor &&rhs) noexcept :
    coroutine_handle_{std::exchange(rhs.coroutine_handle_, {})} {}
  ~CoroutineHandleFunctor() noexcept {
    if (static_cast<bool>(coroutine_handle_))
      coroutine_handle_.destroy();
  }

 public:
  void execute_destroy() noexcept {
    auto handle = std::exchange(coroutine_handle_, {});
    handle();
  }

 public:
  void operator()() noexcept {
    execute_destroy();
  }
 
 private:
  coroutine_handle<void> coroutine_handle_;

};

}

} // namespace plain::concurrency

using plain::concurrency::CoroutineHandleFunctor;

void Task::build(Task &&rhs) noexcept {
  vtable_ = std::exchange(rhs.vtable_, nullptr);
  if (vtable_ == nullptr) return;
  
  if (contains<CoroutineHandleFunctor>(vtable_)) {
    return CallableVtable<CoroutineHandleFunctor>::move_destroy(
      rhs.buffer_, buffer_);
  }

  if (contains<AwaitViaFunctor>(vtable_)) {
    return CallableVtable<AwaitViaFunctor>::move_destroy(rhs.buffer_, buffer_);
  }

  const auto move_destroy_fn = vtable_->move_destroy_fn;
  if (vtable::trivially_copiable_destructible(move_destroy_fn)) {
    std::memcpy(buffer_, rhs.buffer_, detail::task_constants::buffer_size);
    return;
  }
  move_destroy_fn(rhs.buffer_, buffer_);
}

void Task::build(coroutine_handle<void> handle) noexcept {
  build(CoroutineHandleFunctor{handle});
}

bool Task::contains_coroutine_handle() const noexcept {
  return contains<CoroutineHandleFunctor>();
}

Task::Task() noexcept : buffer_{}, vtable_{nullptr} {}

Task::Task(Task &&rhs) noexcept {
  build(std::move(rhs));
}

Task::Task(coroutine_handle<void> handle) noexcept {
  build(handle);
}

Task::~Task() noexcept {
  clear();
}

void Task::operator()() {
  const auto vtable = std::exchange(vtable_, nullptr);
  if (vtable == nullptr) return;
  if (contains<CoroutineHandleFunctor>(vtable)) {
    return CallableVtable<CoroutineHandleFunctor>::execute_destroy(buffer_);
  }
  if (contains<AwaitViaFunctor>(vtable)) {
    return CallableVtable<AwaitViaFunctor>::execute_destroy(buffer_);
  }
  vtable->execute_destroy_fn(buffer_);
}

Task &Task::operator=(Task &&rhs) noexcept {
  if (&rhs == this) return *this;
  clear();
  build(std::move(rhs)); //FIXME: need std::move with a rvalue ?
  return *this;
}

void Task::clear() noexcept {
  if (vtable_ == nullptr) return;
  const auto vtable = std::exchange(vtable_, nullptr);
  
  if (contains<CoroutineHandleFunctor>(vtable)) {
    return CallableVtable<CoroutineHandleFunctor>::destroy(buffer_);
  }
  
  if (contains<AwaitViaFunctor>(vtable)) {
    return CallableVtable<AwaitViaFunctor>::destroy(buffer_);
  }

  auto destroy_fn = vtable->destroy_fn;
  if (vtable::trivially_destructible(destroy_fn))
    return;
  destroy_fn(buffer_);
}

Task::operator bool() const noexcept {
  return vtable_ != nullptr;
}
