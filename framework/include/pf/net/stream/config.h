/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id config.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/06/26 12:53
 * @uses your description
*/
#ifndef PF_NET_STREAM_CONFIG_H_
#define PF_NET_STREAM_CONFIG_H_

#include "pf/net/config.h"

#define NETINPUT_BUFFERSIZE_DEFAULT (64*1024) //default size
#define NETINPUT_DISCONNECT_MAXSIZE (96*1024) //if buffer more than it, disconnet.
#define NETOUTPUT_BUFFERSIZE_DEFAULT (8*1024)
#define NETOUTPUT_DISCONNECT_MAXSIZE (100*1024)

namespace pf_net {

namespace stream {

class Basic;
class Input;
class Output;
class Encryptor;
class Compressor;

};

}; //namespace pf_net

#endif //PF_NET_STREAM_CONFIG_H_
