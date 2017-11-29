/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id db_query.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/01/07 11:27
 * @uses The db query packet.
*/
#ifndef PF_CACHE_PACKET_DB_QUERY_H_
#define PF_CACHE_PACKET_DB_QUERY_H_

#include "pf/cache/packet/config.h"
#include "pf/basic/string.h"
#include "pf/net/packet/interface.h"
#include "pf/net/packet/factory.h"
#include "pf/db/config.h"

namespace pf_cache {

namespace packet {

class DBQuery : public pf_net::packet::Interface {

 public:
   DBQuery() : id_{0},
     type_{0},
     operate_{0},
     key_{0},
     sql_str_{0} {}
   virtual ~DBQuery() {}

 public:
   virtual bool read(pf_net::stream::Input &);
   virtual bool write(pf_net::stream::Output &);
   virtual uint32_t execute(pf_net::connection::Basic *connection);
   uint16_t get_id() const { return id_; };
   virtual uint32_t size() const;
   virtual void set_id(uint16_t id) { id_ = id; };
   void set_operate(int8_t operate) { operate_ = operate; }
   int8_t get_operate() const { return operate_; }
   void set_type(int8_t type) { type_ = type; }
   int8_t get_type() const { return type_; }
   void set_sql_str(const std::string &str) {
     pf_basic::string::safecopy(sql_str_, str.c_str(), sizeof(sql_str_ - 1));
   }
   const char *get_sql_str() {
     return sql_str_;
   }

 private:
   uint16_t id_;
   int8_t type_;
   int8_t operate_;
   char key_[128];
   char sql_str_[SQL_LENGTH_MAX];

};

class DBQueryFactory : public pf_net::packet::Factory {

 public:
   DBQueryFactory() : id_{0} {}
   virtual ~DBQueryFactory() {}

 public:
   void set_id(uint16_t id) { id_ = id; }
   
 public:
   virtual pf_net::packet::Interface *packet_create() {
     return new DBQuery();
   }
   uint16_t packet_id() const {
     return id_;
   }
   virtual uint32_t packet_max_size() const;

 private:
   uint16_t id_;

};

}; //namespace packet

}; //namespace pf_cache

#endif //PF_CACHE_PACKET_DB_QUERY_H_
