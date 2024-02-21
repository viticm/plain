#include "plain/net/connection/io_uring.h"
#if __has_include(<liburing.h>)
#include <liburing.h>
#include <sys/eventfd.h>
#include <sys/utsname.h>
#include <sys/poll.h>
// #define PLAIN_LIBURING_ENABLE
#endif
#include "plain/basic/logger.h"
#include "plain/basic/utility.h"

using plain::net::connection::IoUring;
using plain::net::detail::Awaitable;

#ifndef PLAIN_LIBURING_ENABLE

struct IoUring::Impl{};

IoUring::IoUring(const setting_t &setting) : Manager(setting) {

}

IoUring::IoUring(
  std::unique_ptr<concurrency::executor::Basic> &&executor,
  const setting_t &setting) :
  Manager(std::forward<decltype(executor)>(executor), setting) {

}

IoUring::~IoUring() = default;

bool IoUring::prepare() noexcept {
  LOG_ERROR << "disabled";
  return false;
}

bool IoUring::work() noexcept {
  LOG_ERROR << "disabled";
  return false;
}

void IoUring::off() noexcept {

}

bool IoUring::sock_add(
  socket::id_t, connection::id_t) noexcept {
  LOG_ERROR << "disabled";
  return false;
}

bool IoUring::sock_remove(socket::id_t) noexcept {
  LOG_ERROR << "disabled";
  return false;
}

void IoUring::handle_input() noexcept {

}
  
void *IoUring::get_sock_data() noexcept {
  return nullptr;
}

plain::net::detail::Task<int32_t> IoUring::accept_await() noexcept {
  co_await Awaitable{nullptr};
}
#else

#ifdef PLAIN_LIBURING_VERBOSE
#define puts_if_verbose(x) puts(x)
#define printf_if_verbose(...) printf(__VA_ARGS__)
#else
#define puts_if_verbose(x)
#define printf_if_verbose(...)
#endif

struct IoUring::Impl {
  Impl();
  ~Impl();
  struct io_uring ring;
  uint32_t cqe_count{0};
  bool ready{false};
  static std::array<bool, IORING_OP_LAST> probe_ops;
  static Awaitable await_work(io_uring_sqe *sqe, uint8_t flags);
  void test_uring_op(const io_uring_params &params) noexcept;
  [[nodiscard]] struct io_uring_sqe *get_sqe() noexcept;
  bool submit() noexcept;
};

std::array<bool, IORING_OP_LAST> IoUring::Impl::probe_ops = {};

IoUring::Impl::Impl() = default;

IoUring::Impl::~Impl() {
  if (ready) io_uring_queue_exit(&ring);
}

Awaitable IoUring::Impl::await_work(io_uring_sqe *sqe, uint8_t flags = 0) {
  io_uring_sqe_set_flags(sqe, flags);
  auto set_data = [sqe](void *data) {
    io_uring_sqe_set_data(sqe, data);
  };
  return Awaitable(set_data);
}

void IoUring::Impl::test_uring_op(const io_uring_params &params) noexcept {
  auto probe = io_uring_get_probe_ring(&ring);
  scoped_executor_t free_probe([=]() { io_uring_free_probe(probe); });
#define TEST_URING_OP(opcode) do {\
    for (int i = 0; i < probe->ops_len; ++i) {\
        if (probe->ops[i].op == opcode && \
            probe->ops[i].flags & IO_URING_OP_SUPPORTED) {\
                probe_ops[i] = true;\
                puts_if_verbose("\t" #opcode);\
                break;\
            }\
        }\
    } while (false)
    puts_if_verbose("Supported io_uring opcodes by current kernel:");
    TEST_URING_OP(IORING_OP_NOP);
    TEST_URING_OP(IORING_OP_READV);
    TEST_URING_OP(IORING_OP_WRITEV);
    TEST_URING_OP(IORING_OP_FSYNC);
    TEST_URING_OP(IORING_OP_READ_FIXED);
    TEST_URING_OP(IORING_OP_WRITE_FIXED);
    TEST_URING_OP(IORING_OP_POLL_ADD);
    TEST_URING_OP(IORING_OP_POLL_REMOVE);
    TEST_URING_OP(IORING_OP_SYNC_FILE_RANGE);
    TEST_URING_OP(IORING_OP_SENDMSG);
    TEST_URING_OP(IORING_OP_RECVMSG);
    TEST_URING_OP(IORING_OP_TIMEOUT);
    TEST_URING_OP(IORING_OP_TIMEOUT_REMOVE);
    TEST_URING_OP(IORING_OP_ACCEPT);
    TEST_URING_OP(IORING_OP_ASYNC_CANCEL);
    TEST_URING_OP(IORING_OP_LINK_TIMEOUT);
    TEST_URING_OP(IORING_OP_CONNECT);
    TEST_URING_OP(IORING_OP_FALLOCATE);
    TEST_URING_OP(IORING_OP_OPENAT);
    TEST_URING_OP(IORING_OP_CLOSE);
    TEST_URING_OP(IORING_OP_FILES_UPDATE);
    TEST_URING_OP(IORING_OP_STATX);
    TEST_URING_OP(IORING_OP_READ);
    TEST_URING_OP(IORING_OP_WRITE);
    TEST_URING_OP(IORING_OP_FADVISE);
    TEST_URING_OP(IORING_OP_MADVISE);
    TEST_URING_OP(IORING_OP_SEND);
    TEST_URING_OP(IORING_OP_RECV);
    TEST_URING_OP(IORING_OP_OPENAT2);
    TEST_URING_OP(IORING_OP_EPOLL_CTL);
    TEST_URING_OP(IORING_OP_SPLICE);
    TEST_URING_OP(IORING_OP_PROVIDE_BUFFERS);
    TEST_URING_OP(IORING_OP_REMOVE_BUFFERS);
    TEST_URING_OP(IORING_OP_TEE);
    TEST_URING_OP(IORING_OP_SHUTDOWN);
    TEST_URING_OP(IORING_OP_RENAMEAT);
    TEST_URING_OP(IORING_OP_UNLINKAT);
    TEST_URING_OP(IORING_OP_MKDIRAT);
    TEST_URING_OP(IORING_OP_SYMLINKAT);
    TEST_URING_OP(IORING_OP_LINKAT);
    TEST_URING_OP(IORING_OP_MSG_RING);
    TEST_URING_OP(IORING_OP_FSETXATTR);
    TEST_URING_OP(IORING_OP_SETXATTR);
    TEST_URING_OP(IORING_OP_FGETXATTR);
    TEST_URING_OP(IORING_OP_GETXATTR);
    TEST_URING_OP(IORING_OP_SOCKET);
    TEST_URING_OP(IORING_OP_URING_CMD);
    TEST_URING_OP(IORING_OP_SEND_ZC);
    TEST_URING_OP(IORING_OP_SENDMSG_ZC);
#undef TEST_URING_OP

#define TEST_URING_FEATURE(feature) \
    if (params.features & feature) puts_if_verbose("\t" #feature)
    puts_if_verbose("Supported io_uring features by current kernel:");
    TEST_URING_FEATURE(IORING_FEAT_SINGLE_MMAP);
    TEST_URING_FEATURE(IORING_FEAT_NODROP);
    TEST_URING_FEATURE(IORING_FEAT_SUBMIT_STABLE);
    TEST_URING_FEATURE(IORING_FEAT_RW_CUR_POS);
    TEST_URING_FEATURE(IORING_FEAT_CUR_PERSONALITY);
    TEST_URING_FEATURE(IORING_FEAT_FAST_POLL);
    TEST_URING_FEATURE(IORING_FEAT_POLL_32BITS);
    TEST_URING_FEATURE(IORING_FEAT_SQPOLL_NONFIXED);
    TEST_URING_FEATURE(IORING_FEAT_EXT_ARG);
    TEST_URING_FEATURE(IORING_FEAT_NATIVE_WORKERS);
    TEST_URING_FEATURE(IORING_FEAT_RSRC_TAGS);
    TEST_URING_FEATURE(IORING_FEAT_CQE_SKIP);
    TEST_URING_FEATURE(IORING_FEAT_LINKED_FILE);
    TEST_URING_FEATURE(IORING_FEAT_REG_REG_RING);
#undef TEST_URING_FEATURE

}

struct io_uring_sqe *IoUring::Impl::get_sqe() noexcept {
  auto r = io_uring_get_sqe(&ring);
  if (!!r) [[likely]]
    return r;

  if (submit())
    r = io_uring_get_sqe(&ring);
  return r;
}

bool IoUring::Impl::submit() noexcept {
  io_uring_cq_advance(&ring, cqe_count);
  cqe_count = 0;
  auto error = io_uring_submit(&ring);
  if (error < 0) {
    LOG_ERROR << "error: " << -error;
    return false;
  }
  return true;
}

IoUring::IoUring(const setting_t &setting) :
  Manager(setting), impl_{std::make_unique<Impl>()} {

}

IoUring::IoUring(
  std::unique_ptr<concurrency::executor::Basic> &&executor,
  const setting_t &setting) :
  Manager(std::forward<decltype(executor)>(executor), setting),
  impl_{std::make_unique<Impl>()} {
}

IoUring::~IoUring() = default;

bool IoUring::prepare() noexcept {
  io_uring_params params {
    .flags = 0,
    .wq_fd = 0
  };
  int32_t error{0};
  if ((error = io_uring_queue_init_params(
      setting_.max_count, &impl_->ring, &params)) < 0) {
    LOG_ERROR << "io_uring_queue_init_params error: " << -error;
    return false;
  }
  impl_->test_uring_op(params);
  impl_->ready = true;
  return true;
}

bool IoUring::work() noexcept {
  for (;;) {
    accept_await(); // accept.
                    // recv ?
    io_uring_submit_and_wait(&impl_->ring, 1);
    io_uring_cqe *cqe{nullptr};
    uint32_t head{0};
    io_uring_for_each_cqe(&impl_->ring, head, cqe) {
      ++impl_->cqe_count;
      auto coro =
        static_cast<net::detail::Resolver *>(io_uring_cqe_get_data(cqe));
      if (coro) coro->resolve(cqe->res);
    }
    if (!running()) break;
  }
  return true;
}

void IoUring::off() noexcept {

}

bool IoUring::sock_add(
  socket::id_t sock_id, connection::id_t conn_id) noexcept {
  auto sqe = impl_->get_sqe();
  io_uring_prep_poll_add(sqe, sock_id, POLLIN);
  return true;
}

bool IoUring::sock_remove(socket::id_t sock_id) noexcept {
  auto sqe = impl_->get_sqe();
  io_uring_prep_poll_remove(sqe, sock_id);
  return true;
}

void IoUring::handle_input() noexcept {

}

void *IoUring::get_sock_data() noexcept {
  return impl_->get_sqe();
}

plain::net::detail::Task<int32_t>
IoUring::accept_await() noexcept {
  auto sqe = impl_->get_sqe();
  io_uring_prep_accept(sqe, listen_fd_, nullptr, nullptr, 0);
  auto fd = co_await impl_->await_work(sqe);
  if (fd != socket::kInvalidId) {
    accept(fd);
  }
}

#endif
