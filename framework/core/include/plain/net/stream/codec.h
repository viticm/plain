/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id codec.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/11/20 17:52
 * @uses The net stream encode and decode class implemention.
 */

#ifndef PLAIN_NET_STREAM_CODEC_H_
#define PLAIN_NET_STREAM_CODEC_H_

#include "plain/net/stream/config.h"
#include "plain/basic/type/config.h"
#include "plain/basic/error.h"

namespace plain::net {
namespace stream {

using encode_func =
  std::function<bytes_t(std::shared_ptr<packet::Basic> packet)>;
using decode_func =
  std::function<error_or_t<std::shared_ptr<packet::Basic>>(
    Basic *input, const packet::limit_t &packet_limit)>;

struct codec_struct {
  encode_func encode{};
  decode_func decode{};
};

using codec_t = codec_struct;

// The default codec functions.
bytes_t encode(std::shared_ptr<packet::Basic> packet);
error_or_t<std::shared_ptr<packet::Basic>>
decode(Basic *input, const packet::limit_t &packet_limit);

bytes_t line_encode(std::shared_ptr<packet::Basic> packet);
error_or_t<std::shared_ptr<packet::Basic>>
line_decode(Basic *input, const packet::limit_t &packet_limit);

} // namespace stream
} // namespace plain::net

#endif // PLAIN_NET_STREAM_CODEC_H_
