#include "pf/basic/string.h"
#include "pf/basic/logger.h"
#include "pf/basic/io.tcc"
#include "pf/net/socket/basic.h"

namespace pf_net {

namespace socket {

Basic::Basic() :
  id_{SOCKET_INVALID},
  host_{0,},
  port_{0} {
  //do nothing.
}

Basic::Basic(const char *_host, uint16_t _port) {
  using namespace pf_basic;
  memset(host_, '\0', sizeof(host_));
  if (_host != nullptr) string::safecopy(host_, _host, sizeof(host_));      
  port_ = _port;
  create();
}

Basic::~Basic() {
  close();
}

bool Basic::create() {
  bool result = true;
  id_ = api::socketex(AF_INET, SOCK_STREAM, 0);
  result = is_valid();
  return result;
}

void Basic::close() {
  if (is_valid() && !error())
    api::closeex(id_);
  id_ = SOCKET_INVALID;
  memset(host_, '\0', sizeof(host_));
  port_ = 0;
}

bool Basic::connect() {
  bool result = true;
  struct sockaddr_in connect_sockaddr_in;
  memset(&connect_sockaddr_in, 0, sizeof(connect_sockaddr_in));
  connect_sockaddr_in.sin_family = AF_INET;
  connect_sockaddr_in.sin_addr.s_addr = inet_addr(host_);
#if OS_WIN
if (0 == strcmp(host_, "0.0.0.0")) {
    connect_sockaddr_in.sin_addr.s_addr = inet_addr("127.0.0.1");
}
#endif
  connect_sockaddr_in.sin_port = htons(port_);
  result = api::connectex(
      id_, 
      reinterpret_cast<const struct sockaddr*>(&connect_sockaddr_in), 
      sizeof(connect_sockaddr_in));
  return result;
}

bool Basic::connect(const char *_host, uint16_t _port) {
  using namespace pf_basic;
  bool result = true;
  if (_host != nullptr)
    string::safecopy(host_, _host, sizeof(host_));
  port_ = _port;
  result = connect();
  return result;
}

bool Basic::reconnect(const char *_host, uint16_t _port) {
  using namespace pf_basic;
  bool result = true;
  close();
  if (_host != nullptr)
    string::safecopy(host_, _host, sizeof(host_));
  port_ = _port;
  create();
  result = connect();
  return result;
}

int32_t Basic::send(const void *buffer, uint32_t length, uint32_t flag) {
  int32_t result = 0;
  result = api::sendex(id_, buffer, length, flag);
  return result;
}

int32_t Basic::receive(void *buffer, uint32_t length, uint32_t flag) {
  int32_t result = 0;
  result = api::recvex(id_, buffer, length, flag);
  return result;
}

uint32_t Basic::available() const {
    uint32_t result = 0;
    result = api::availableex(id_);
    return result;
}

int32_t Basic::accept(struct sockaddr_in *accept_sockaddr_in) {
  int32_t result = SOCKET_ERROR;
  uint32_t addrlength = 0;
  addrlength = sizeof(struct sockaddr_in);
  result = api::acceptex(
      id_, 
      reinterpret_cast<struct sockaddr *>(accept_sockaddr_in),
      &addrlength);
  return result;
}

bool Basic::bind(const char *ip) {
  using namespace pf_basic;
  bool result = true;
  struct sockaddr_in connect_sockaddr_in;
  connect_sockaddr_in.sin_family = AF_INET;
  if (nullptr == ip || 
      0 == strlen(ip) || 
      0 == strcmp("127.0.0.1", ip) ||
      0 == strcmp("0.0.0.0", ip)) {
    connect_sockaddr_in.sin_addr.s_addr = htonl(INADDR_ANY);
  } else {
    connect_sockaddr_in.sin_addr.s_addr = inet_addr(ip);
  }
  connect_sockaddr_in.sin_port = htons(port_);
  result = api::bindex(
      id_, 
      reinterpret_cast<const struct sockaddr*>(&connect_sockaddr_in), 
      sizeof(connect_sockaddr_in));
  if (0 == port_) {
    int32_t inlength = sizeof(connect_sockaddr_in);
    if (SOCKET_ERROR == 
        api::getsockname_ex(id_, 
          reinterpret_cast<struct sockaddr*>(&connect_sockaddr_in), 
          &inlength)) {
      io_cerr("[net.socket] (socket::Basic::bind) error, can't get port");
      return false;
    }
    port_ = ntohs(connect_sockaddr_in.sin_port);
  }
  return result;
}

bool Basic::bind(uint16_t _port, const char *ip) {
  bool result = true;
  port_ = _port;
  result = bind(ip);
  return result;
}

bool Basic::listen(uint32_t backlog) {
  bool result = true;
  result = api::listenex(id_, backlog);
  return result;
}

int32_t Basic::select(int32_t maxfdp, 
                     fd_set *readset, 
                     fd_set *writeset, 
                     fd_set *exceptset,
                     timeval *timeout) {
  int32_t result = SOCKET_ERROR;
  result = api::selectex(maxfdp, readset, writeset, exceptset, timeout);
  return result;
}

uint32_t Basic::get_linger() const {
  uint32_t result = 0;
  struct linger getlinger;
  uint32_t length = sizeof(getlinger);
  api::getsockopt_exb(id_, SOL_SOCKET, SO_LINGER, &getlinger, &length);
  result = getlinger.l_linger;
  return result;
}

bool Basic::set_linger(uint32_t lingertime) {
  bool result = true;
  struct linger setlinger;
  setlinger.l_onoff = lingertime > 0 ? 1 : 0;
  setlinger.l_linger = static_cast<uint16_t>(lingertime);
  result = api::setsockopt_ex(id_, 
                              SOL_SOCKET, 
                              SO_LINGER, 
                              &setlinger, 
                              sizeof(setlinger));
  return result;
}

bool Basic::is_reuseaddr() const {
    bool result = true;
    int32_t reuse = 0;
    uint32_t length = sizeof(reuse);
    result = api::getsockopt_exb(id_, 
                                 SOL_SOCKET, 
                                 SO_REUSEADDR, 
                                 &reuse, 
                                 &length);
    return result;
}

bool Basic::set_reuseaddr(bool on) {
  bool result = true;
  int32_t option = true == on ? 1 : 0;
  result = api::setsockopt_ex(id_, 
                              SOL_SOCKET, 
                              SO_REUSEADDR, 
                              &option, 
                              sizeof(option));
  return result;
}

uint32_t Basic::get_last_error_code() const {
  uint32_t result = 0;
  result = api::getlast_errorcode();
  return result;
}

void Basic::get_last_error_message(char *buffer, uint16_t length) const {
  api::getlast_errormessage(buffer, length);
}

bool Basic::error() const {
  bool result = true;
  int32_t option_value = 0;
  uint32_t option_length = sizeof(option_value);
  api::getsockopt_exu(id_, 
                      SOL_SOCKET, 
                      SO_ERROR, 
                      &option_value,
                      &option_length);
  result = 0 == option_value ? false : true;
  return result;
}

bool Basic::is_nonblocking() const {
  bool result = true;
  result = api::get_nonblocking_ex(id_);
  return result;
}

bool Basic::set_nonblocking(bool on) {
  bool result = true;
  result = api::set_nonblocking_ex(id_, on);
  return result;
}

uint32_t Basic::get_receive_buffer_size() const {
  uint32_t option_value = 0;
  uint32_t option_length = sizeof(option_value);
  api::getsockopt_exb(id_, 
                      SOL_SOCKET, 
                      SO_RCVBUF, 
                      &option_value, 
                      &option_length);
  return option_value;
}

bool Basic::set_receive_buffer_size(uint32_t size) {
  bool result = true;
  result = 
    api::setsockopt_ex(id_, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
  return result;
}

uint32_t Basic::get_send_buffer_size() const {
  uint32_t option_value = 0;
  uint32_t option_length = sizeof(option_value);
  api::getsockopt_exb(id_, 
                      SOL_SOCKET, 
                      SO_SNDBUF, 
                      &option_value, 
                      &option_length);
  return option_value;
}

bool Basic::set_send_buffer_size(uint32_t size) {
  bool result = true;
  result = 
    api::setsockopt_ex(id_, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
  return result;
}

uint64_t Basic::uint64host() const {
  uint64_t result = 0;
  if (0 == strlen(host_)) {
    result = static_cast<uint64_t>(htonl(INADDR_ANY));
  } else {
    result = static_cast<uint64_t>(inet_addr(host_));
  }
  return result;
}

} //namespace socket

} //namespace pf_net
