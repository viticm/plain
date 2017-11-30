/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id connection_resolver.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/11/30 16:08
 * @uses your description
*/
#ifndef PF_DB_CONNECTION_RESOLVER_H_
#define PF_DB_CONNECTION_RESOLVER_H_

#include "pf/db/config.h"
#include "pf/db/connection_resolver_interface.h"

namespace pf_db {

class PF_API ConnectionResolver : public ConnectionResolverInterface {

 public:
   ConnectionResolver(connection_set_t connections = {}) {
     auto it = connections.begin();
     for (; it != connections.end(); ++it) 
       connections_[it->first] = std::move(it->second);
   };
   virtual ~ConnectionResolver() {};

 public:
   
   //Add a connection to the resolver.
   void add_connection(
       const std::string &name, ConnectionInterface *connection) {
     std::unique_ptr< ConnectionInterface > temp(connection);
     connections_[name] = std::move(temp);
   };

   //Check if a connection has been registered.
   bool has_connection(const std::string &name) {
     return connections_.find(name) != connections_.end();
   };

 public:
   
   //Get a database connection instance.
   ConnectionInterface *connection(const std::string &name) {
     auto it = connections_.find(name);
     return it == connections_.end() ? nullptr : it->second.get();
   };

   //Get the default connection name.
   const std::string get_default_connection() const {
     return default_;
   };

   //Set the default connection name.
   void set_default_connection(const std::string &name) {
     default_ = name;
   };

 public:
   using connection_set_t = 
     std::map< int32_t, std::unique_ptr< ConnectionInterface > >;

 protected:

   //The default connection name.
   std::string default_;

   //All of the registered connections.
   connection_set_t connections_;

}

};

#endif //PF_DB_CONNECTION_RESOLVER_H_
