/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id dispatcher.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2024 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2024/09/11 10:43
 * @uses The net rpc dispatcher class implemention.
 */

#ifndef PLAIN_NET_RPC_DISPATCHER_H_
#define PLAIN_NET_RPC_DISPATCHER_H_

#include "plain/net/rpc/config.h"
#include <unordered_map>
#include "plain/net/rpc/traits.h"
#include "plain/net/rpc/utility.h"

namespace plain::net {
namespace rpc {

class PLAIN_API Dispatcher {

 public:
  using adaptor_type =
    std::function<std::shared_ptr<Packer>(Unpacker &pack)>;

 public:
  template <typename F> void bind(std::string const &name, F func) {
    enforce_unique_name(name);
    bind(
      name, func, typename func_kind_info<F>::result_kind(),
      typename func_kind_info<F>::args_kind());
  }

  template <typename F>
  void bind(
    std::string const &name, F func, tags::void_result const &,
    tags::zero_arg const &) {
    enforce_unique_name(name);
    funcs_.insert(
      std::make_pair(name, [func, name](Unpacker &args) {
      //enforce_arg_count(name, 0, args.via.array.size);
      func();
      return std::shared_ptr<Packer>{};
    }));
  }

  template <typename F>
  void bind(
    std::string const &name, F func, tags::void_result const &,
    tags::nonzero_arg const &) {
    using args_type = typename func_traits<F>::args_type;

    enforce_unique_name(name);
    funcs_.insert(
      std::make_pair(name, [func, name](Unpacker &args) {
      constexpr int args_count = std::tuple_size<args_type>::value;
      //enforce_arg_count(name, args_count, args.via.array.size);
      args_type args_real;
      args.unpack(args_real);
      call(func, args_real);
      return std::shared_ptr<Packer>{};
    }));
  }

  template <typename F>
  void bind(
    std::string const &name, F func, tags::nonvoid_result const &,
    tags::zero_arg const &) {

    enforce_unique_name(name);
    funcs_.insert(
      std::make_pair(name, [func, name](Unpacker &args) {
      //enforce_arg_count(name, 0, args.via.array.size);
      auto r = func();
      auto packer = std::make_shared<Packer>();
      packer->process(r);
      return packer;
    }));
  }

  template <typename F>
  void bind(
    std::string const &name, F func, tags::nonvoid_result const &,
    tags::nonzero_arg const &) {
    using args_type = typename func_traits<F>::args_type;

    enforce_unique_name(name);
    funcs_.insert(
      std::make_pair(name, [func,name](Unpacker &args) {
      constexpr uint32_t args_count = std::tuple_size<args_type>::value;
      //enforce_arg_count(name, args_count, args.via.array.size);
      args_type args_real;
      args.process(args_real);
      auto r = call(func, args_real);
      auto packer = std::make_shared<Packer>();
      packer->process(r);
      return packer;
    }));
  }

  void unbind(std::string const &name) {
    funcs_.erase(name);
  }

  std::vector<std::string> names() const {
    std::vector<std::string> names;
    for(auto it = funcs_.begin(); it != funcs_.end(); ++it)
      names.push_back(it->first);
    return names;
  }

 public:
  error_or_t<std::shared_ptr<Packer>>
  dispatch(std::shared_ptr<packet::Basic> packet);

 private:
  void enforce_unique_name(const std::string &name);

 private:
  std::unordered_map<std::string, adaptor_type> funcs_;

};

} // namespace rpc
} // namespace plain::net

#endif // PLAIN_NET_RPC_DISPATCHER_H_
