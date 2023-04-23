/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id ring.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/04/04 18:40
 * @uses The ring buffer class(lock free).
 *       FixedRing: allocate memory from stack(T buffer[N])
 *       DynamicRing: allocate memory from heap(std::vector<T>)
 */

#ifndef PLAIN_BASIC_RING_H_
#define PLAIN_BASIC_RING_H_

#include "plain/basic/config.h"
#include "plain/basic/concepts.h"

namespace plain {

template <typename T, bool fake_tso = false>
class Ring : noncopyable {
using cookie_func_t = std::function<void()>;

 public:
  Ring()
    : cookie_{nullptr}, head_{0}, tail_{0}, size_{0},mask_{0},
    buffer_{nullptr} {
    set_cookie(cookie_start);
  }
  virtual ~Ring() {
    set_cookie(cookie_end);
  }
  Ring(Ring &&) = default;
  Ring &operator=(Ring &&) = default;

 public:
  bool can_insert() noexcept {
    std::size_t tmp_head{head_.load(std::memory_order_relaxed)};
    if ((tmp_head - tail_.load(index_acquire_barrier)) == size_ &&
        !resize(size_ << 1)) {
      return false;
    }
    return true;
  }
  bool insert(T data) {
    std::size_t tmp_head{head_.load(std::memory_order_relaxed)};

    if (!can_insert()) {
      return false;
    } else {
      buffer_[tmp_head++ & mask_] = data;
      std::atomic_signal_fence(std::memory_order_release);
      head_.store(tmp_head, index_release_barrier);
    }
    return true;
  }
  bool insert(const T *data) {
    std::size_t tmp_head{head_.load(std::memory_order_relaxed)};

    if (!can_insert()) {
      return false;
    } else {
      buffer_[tmp_head++ & mask_] = *data;
      std::atomic_signal_fence(std::memory_order_release);
      head_.store(tmp_head, index_release_barrier);
    }
    return true;
  }

  bool remove(T &data) {
    return remove(&data);
  }
  bool remove(T *data) {
    std::size_t tmp_tail = tail_.load(std::memory_order_relaxed);
    if (tmp_tail == head_.load(index_acquire_barrier)) {
      return false;
    } else {
      *data = buffer_[tmp_tail++ & mask_];
			std::atomic_signal_fence(std::memory_order_release);
			tail_.store(tmp_tail, index_release_barrier);
    }
    return true;
  }
  std::size_t remove(std::size_t count) {
    std::size_t tmp_tail{tail_.load(std::memory_order_relaxed)};
    std::size_t avail{head_.load(std::memory_order_relaxed) - tmp_tail};

    count = (count > avail) ? avail : count;

    tail_.store(tmp_tail + count, index_release_barrier);
    return count;
	}

  std::size_t write(const T *buffer, std::size_t count) noexcept {
    std::size_t available{0};
	  std::size_t tmp_head{head_.load(std::memory_order_relaxed)};
    std::size_t to_write{count};

		available = size_ - (tmp_head - tail_.load(index_acquire_barrier));

    // Not enough then try resize.
    if (available < count && resize(size_ << 1)) {
      available = size_ - (tmp_head - tail_.load(index_acquire_barrier));
    }

		if (available < count) // do not write more than we can
			to_write = available;

		// maybe divide it into 2 separate writes
		for (std::size_t i = 0; i < to_write; i++)
			buffer_[tmp_head++ & mask_] = buffer[i];

		std::atomic_signal_fence(std::memory_order_release);
		head_.store(tmp_head, index_release_barrier);

		return to_write;
  }
  std::size_t read(T *buffer, std::size_t count) noexcept {
    std::size_t available{0};
    std::size_t tmp_tail{tail_.load(std::memory_order_relaxed)};
    std::size_t to_read {count};

		available = head_.load(index_acquire_barrier) - tmp_tail;

		if (available < count) // do not read more than we can
			to_read = available;

		// maybe divide it into 2 separate reads
		for (std::size_t i = 0; i < to_read; ++i)
			buffer[i] = buffer_[tmp_tail++ & mask_];

		std::atomic_signal_fence(std::memory_order_release);
		tail_.store(tmp_tail, index_release_barrier);

		return to_read;
  }

 public:
  std::size_t write_avail() const noexcept {
    return size_ -
      (head_.load(std::memory_order_relaxed) -
       tail_.load(index_acquire_barrier));
  }
  std::size_t read_avail() const noexcept {
    return head_.load(index_acquire_barrier) -
      tail_.load(std::memory_order_relaxed);
  }

  bool full() const noexcept {
    return 0 == write_avail();
  }
  bool empty() const noexcept {
    return 0 == read_avail();
  }

  void producer_clear() noexcept {
    comsumer_clear();
  }
  void comsumer_clear() noexcept {
    tail_.store(
        head_.load(std::memory_order_relaxed), std::memory_order_relaxed);
  }

  // Gets the first element in the buffer on consumed side.
  T *peek(std::size_t count = 0) {
    std::size_t avail = read_avail();
		if (avail < count) {
			return nullptr;
    } else {
      std::size_t tmp_tail{tail_.load(std::memory_order_relaxed)};
			return &buffer_[tmp_tail & mask_];
    }
  }

  T *at(std::size_t index) {
    std::size_t tmp_tail{tail_.load(std::memory_order_relaxed)};

    if ((head_.load(index_acquire_barrier) - tmp_tail) <= index)
      return nullptr;
    else
      return &buffer_[(tmp_tail + index) & mask_];
  }

  T &operator[](size_t index) {
		return buffer_[(tail_.load(std::memory_order_relaxed) + index) & mask_];
	}
  
 protected:
  void set_buffer(T *buffer, std::size_t size) noexcept {
    buffer_ = buffer;
    std::atomic_signal_fence(std::memory_order_release);
    size_.store(size, index_release_barrier);
    mask_.store(size - 1, index_release_barrier);
  }

 private:
  virtual bool resize(size_t) { return false; }

 private:
  void set_cookie(cookie_func_t func) {
    cookie_ = func;
  }

 private:
  static void cookie_start() {}
  static void cookie_end() {}

 private:
  constexpr static std::memory_order index_acquire_barrier = fake_tso 
    ? std::memory_order_relaxed
    : std::memory_order_acquire;
  constexpr static std::memory_order index_release_barrier = fake_tso
    ? std::memory_order_relaxed
    : std::memory_order_release;

 private:
  cookie_func_t cookie_;
  std::atomic<std::size_t> head_;
  std::atomic<std::size_t> tail_;
  std::atomic<std::size_t> size_;
  std::atomic<std::size_t> mask_;
  T *buffer_;

};

template <typename T, std::size_t SIZE, bool fake_tso = false>
requires power_of_two<SIZE>
class FixedRing : public Ring<T, fake_tso> {

 public:
  FixedRing() {
    this->set_buffer(buffer_, SIZE);
  }
  virtual ~FixedRing() = default;

 private:
  T buffer_[SIZE + 1]{0};

};

template <typename T, std::size_t SIZE, bool fake_tso = false>
requires power_of_two<SIZE>
class DynamicRing : public Ring<T, fake_tso> {

 public:
   DynamicRing() {
     buffer_.reserve(SIZE);
     set_buffer(buffer_.data(), SIZE);
   }
   virtual ~DynamicRing() = default;

 private:
  bool resize(std::size_t size) override {
    buffer_.resize(size);
    this->set_buffer(buffer_.data(), size);
    return true;
  }

 private:
  std::vector<T> buffer_; // FIXME: C++20: std::vector is constexpr
                          // 是否可以将FixedRing和DynamicRing做一个整合？

};

} // namespace plain

#endif // PLAIN_BASIC_RING_H_
