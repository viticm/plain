/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id connection_resolver_interface.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/11/30 15:52
 * @uses your description
*/
#ifndef PF_DB_CONNECTION_RESOLVER_INTERFACE_H_
#define PF_DB_CONNECTION_RESOLVER_INTERFACE_H_

#include "pf/db/config.h"

namespace pf_db {

class PF_API ConnectionResolverInterface {

 public:
   ConnectionResolverInterface() {};
   virtual ~ConnectionResolverInterface() {};

 public:
   
   //Get a database connection instance.
   ConnectionInterface *connection(const std::string &name) = 0;

   //Get the default connection name.
   const std::string get_default_connection() const = 0;

   //Set the default connection name.
   void set_default_connection(const std::string &name) = 0;

}

};

#endif //PF_DB_CONNECTION_RESOLVER_INTERFACE_H_
