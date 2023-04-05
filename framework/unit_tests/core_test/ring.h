/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id ring.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/04/04 18:40
 * @uses The ring buffer class.
 *       FixedRing: allocate memory from stack(T buffer[N])
 *       DynamicRing: allocate memory from heap(std::vector<T>)
 */

#ifndef PLAIN_BASIC_RING_H_
#define PLAIN_BASIC_RING_H_

#include <cstdint>
#include <iterator>
#include <iostream>
#include "plain/basic/config.h"

namespace plain {

// Fixed size ring buffer, FIFO
template <typename T>
class Ring {

 public:
  Ring() = default;
  virtual ~Ring() = default;
  Ring(const Ring &) = default;
  Ring &operator=(const Ring &) = default;
  Ring(Ring &&) = default;
  Ring &operator=(Ring &&) = default;

 public:
  template <typename pointer_type, typename reference_type>
  struct iterator_base {
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = T;
    using pointer           = pointer_type;
    using reference         = reference_type;

    constexpr iterator_base(pointer buffer, std::size_t const& ptr,
                            std::size_t const& max, std::size_t const& end)
        : buffer_(buffer), ptr_(ptr), max_(max), m_end(end) {}

    constexpr auto operator*() const -> reference { return buffer_[ptr_]; }
    constexpr auto operator->() -> pointer { return &buffer_[ptr_]; }

    constexpr auto operator++() -> iterator_base& {    // prefix increment
      ptr_ = (ptr_ + max_ - 1) % max_;
      return *this;
    }
    constexpr auto operator++(int) -> iterator_base {  // postfix increment
      auto tmp = *this;
      ++(*this);
      return tmp;
    }

    constexpr friend auto operator==(
      iterator_base const& a, iterator_base const& b) -> bool {
      return a.ptr_ == b.ptr_;
    }
    constexpr friend auto operator!=(
      iterator_base const& a, iterator_base const& b) -> bool {
      return !(a == b);
    }

    // bidirectional iterator requirements
    constexpr auto operator--() -> iterator_base& {
      ptr_ = (ptr_ + 1) % max_;
      return *this;
    }
    constexpr auto operator--(int) -> iterator_base {
      auto tmp = *this;
      --(*this);
      return tmp;
    }

    // random access iterator requirements
    constexpr auto operator+=(
      difference_type const& n) -> iterator_base& {
      if ((n > 0 && n > difference_type(max_ - 1)) ||
          (n < 0 && -n > difference_type(max_ - 1)))  // sets the iterator to
                                                      // end iterator
          ptr_ = (ptr_ + max_ + max_ - 1) % max_;
      else // do normal addition with wrap
          ptr_ = (ptr_ + max_ + n) % max_;
      return *this;
    }

    constexpr auto operator+(difference_type const& n) -> iterator_base {
      if ((n > 0 && n > max_ - 1) || (n < 0 && -n > max_ - 1))  // sets the iterator to end iterator
          ptr_ = (ptr_ + max_ + max_ - 1) % max_;
      else                                                        // do normal addition with wrap
          ptr_ = (ptr_ + max_ + n) % max_;
      return *this;
    }

    constexpr auto operator-(difference_type const& n) -> iterator_base {
      return (*this) + (-n);
    }

    constexpr auto operator-=(difference_type const& n) -> iterator_base& {
      return (*this) += (-n);
    }

    constexpr friend auto operator-(
      iterator_base const& a, iterator_base const& b) -> difference_type {
      // calcuate the distance to the end iterator
      auto a_end = a.dist_to_end();
      auto b_end = b.dist_to_end();
      return a_end - b_end;
    }

    constexpr auto operator[](difference_type const& n) -> reference {
      if ((n > 0 && n > max_ - 1) || (n < 0 && -n > max_ - 1)) // offset the pointer with wrap
        return buffer_[(ptr_ + max_ + n) % max_];
      else                                                       // deference the unused part of the memory,
        return buffer_[(ptr_ + max_ + max_ - 1) % max_];  // i.e the place where end iterator is pointed at
    }

    constexpr friend auto operator<(
        iterator_base const& a, iterator_base const& b) -> bool {
      return b - a > 0;
    }
    constexpr friend auto operator>(
        iterator_base const& a, iterator_base const& b) -> bool {
      return b < a;
    }
    constexpr friend auto operator>=(
        iterator_base const& a, iterator_base const& b) -> bool {
      return !(a < b);
    }
    constexpr friend auto operator<=(
        iterator_base const& a, iterator_base const& b) -> bool {
      return !(a > b);
    }

    constexpr auto ptr() const -> std::size_t { return ptr_; };
   private:
    // FIXME: Find a better way to get the distance from the current ptr to the end, O(n)
    auto dist_to_end() const -> difference_type {
      auto start = ptr_;
      auto dist = 0;
      while(start != m_end) {
        dist++;
        start = (start + max_ - 1) % max_;
      }
      return dist;
    }
   private:
    pointer     buffer_;
    std::size_t ptr_;
    std::size_t max_;
    std::size_t m_end;
  };

  using iterator       = iterator_base<T*, T&>;
  using const_iterator = iterator_base<T const*, T const&>;
  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  auto begin() -> iterator { return iterator(buffer_, tail_, max_, head_); }
  auto end()   -> iterator {
    auto const e = head_ == tail_ ? (head_ + 1) % max_ : head_;
    return iterator(buffer_, e, max_, e);
  }
  auto begin() const -> const_iterator {
    return const_iterator(buffer_, tail_, max_, head_);
  }
  auto end()   const -> const_iterator { 
    auto const e = head_ == tail_ ? (head_ + 1) % max_ : head_;
    return const_iterator(buffer_, e, max_, e);
  }
  auto cbegin() const -> const_iterator { return begin(); }
  auto cend()   const -> const_iterator { return end();   }

  auto rbegin() -> reverse_iterator { return reverse_iterator(end()); }
  auto rend()   -> reverse_iterator { return reverse_iterator(begin()); }
  auto crbegin() const -> const_reverse_iterator {
    return const_reverse_iterator(cend()); 
  }
  
  auto crend()   const -> const_reverse_iterator {
    return const_reverse_iterator(cbegin()); 
  }

 public:

  std::size_t write(const T* buff, std::size_t count);
  std::size_t read(T *buff, std::size_t count);

  auto enq(T const& value) -> void {
    buffer_[head_] = value;
    head_ = Ring::dec_wrap(head_, max_);
    if (head_ == tail_)
      tail_ = Ring::dec_wrap(tail_, max_);
    if (size_ < max_)
      ++size_;
  }

  auto enq_keep(T const& value) -> void {
    if (size_ == max_ - 1) return;
    enq(value);
  }

  auto deq() -> T {
    auto const ret = buffer_[tail_];
    if (tail_ != head_) {
      tail_ = Ring::dec_wrap(tail_, max_);
      --size_;
    }
    return ret;
  }

  constexpr auto capacity() const -> std::size_t { return size_ - 1; }
  auto size() const -> std::size_t { return size_; }
  auto back() -> T { return buffer_[tail_]; }
  auto front() -> T { return buffer_[(head_ + 1) % max_]; }

  using ref_type       = T&;
  using const_ref_type = T const&;
  
  auto operator[](std::size_t const& offset) -> ref_type {
    return at_back(offset);
  }

  auto operator[](std::size_t const& offset) const -> const_ref_type {
    return at_back(offset); 
  }

  auto at_front(std::size_t const& offset) -> ref_type {
    return buffer_[index_front(offset)];
  }
  auto at_front(std::size_t const& offset) const -> const_ref_type {
    return buffer_[index_front(offset)];
  }

  auto at_back(std::size_t const& offset) -> ref_type {
    return buffer_[index_back(offset)];
  }
  auto at_back(std::size_t const& offset) const -> const_ref_type {
    return buffer_[index_back(offset)];
  }

  auto index_front(std::size_t const& offset) const -> std::size_t {
    return (tail_ + max_ - offset) % max_;
  }
  auto index_back(std::size_t const& offset) const -> std::size_t {
    auto index = size_ == 0 ? head_ : head_ + 1;
    return (index + max_ + offset) % max_;
  }

 protected:

  void set_buffer(T *buffer, std::size_t size) {
    buffer_ = buffer;
    size_ = size;
  }

 private:
  
  static auto inc_wrap(
      std::size_t const& value, std::size_t const& max) -> std::size_t {
    return (value + 1) % max;
  }

  static auto dec_wrap(
      std::size_t const& value, std::size_t const& max) -> std::size_t {
    return (value + max - 1) % max;
  }

 private:
  // T buffer_[SIZE + 1];  // allocate extra space for end iterator
  T* buffer_;
  std::size_t max_;
  std::size_t head_;
  std::size_t tail_;
  std::size_t size_;
};

template <typename T, std::size_t SIZE>
class FixedRing : public Ring<T> {

 public:
  FixedRing() {
    this->set_buffer(buffer_, SIZE);
  }
  virtual ~FixedRing() = default;
  FixedRing(const FixedRing &) = default;
  FixedRing &operator=(const FixedRing &) = default;
  FixedRing(FixedRing &&) = default;
  FixedRing &operator=(FixedRing &&) = default;

 private:
  T buffer_[SIZE + 1];

};

template <typename T, std::size_t SIZE>
class DynamicRing : public Ring<T> {

 public:
  DynamicRing() {
    buffer_.reserve(SIZE + 1);
    this->set_buffer(&buffer_[0], SIZE + 1);
  }
  virtual ~DynamicRing() = default;
  DynamicRing(const DynamicRing &) = default;
  DynamicRing &operator=(const DynamicRing &) = default;
  DynamicRing(DynamicRing &&) = default;
  DynamicRing &operator=(DynamicRing &&) = default;

 public:
  void reserve(std::size_t size) {
    buffer_.reserve(size + 1);
    this->set_buffer(&buffer_[0], size + 1);
  }

 private:
  std::vector<T> buffer_;

};

} // namespace plain

#endif // PLAIN_BASIC_RING_H_
