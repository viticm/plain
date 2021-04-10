/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id net_output.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/03/31 15:52
 * @uses The console net output class;
 */

#ifndef PF_CONSOLE_NET_OUTPUT_H_
#define PF_CONSOLE_NET_OUTPUT_H_

#include "pf/console/output.h"
#include "pf/net/connection/config.h"

namespace pf_console {

class NetOutput : public Output {

 public:

   NetOutput(pf_net::connection::Basic *connection) : connection_{connection} {}
   virtual ~NetOutput() {}

 public:
   
   // Writes a message to the output.
   virtual void write(const std::string &messages, 
              bool newline = false, uint16_t options = 0);

 private:

   // The net connection.
   pf_net::connection::Basic *connection_;
   
};

} // namespace pf_console

#endif // PF_CONSOLE_NET_OUTPUT_H_
