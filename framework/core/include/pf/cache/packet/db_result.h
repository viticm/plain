/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id db_result.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/01/06 16:24
 * @uses The cache db query result net packet.
*/
#ifndef PF_CACHE_PACKET_DB_RESULT_H_
#define PF_CACHE_PACKET_DB_RESULT_H_

#include "pf/cache/packet/config.h"
#include "pf/basic/string.h"
#include "pf/net/packet/interface.h"
#include "pf/net/packet/factory.h"
#include "pf/db/config.h"

namespace pf_cache {

namespace packet {

class DBResult : public pf_net::packet::Interface {

 public:
   DBResult() 
     : id_{0},
       result_{kResultFailed},
       operate_{-1},
       key_{0} { data_size_ = get_data_size(); }
   virtual ~DBResult() {}

 public:
   enum {
     kResultFailed = -1,
     kResultSuccess,
   };

 public:
   virtual bool read(pf_net::stream::Input &);
   virtual bool write(pf_net::stream::Output &);
   virtual uint32_t execute(pf_net::connection::Basic *connection);
   uint16_t get_id() const { return id_; };
   virtual uint32_t size() const;
   virtual void set_id(uint16_t id) { id_ = id; };
   void set_operate(int8_t operate) { operate_ = operate; }
   int8_t get_operate() const { return operate_; }
   void set_key(const std::string &key) {
     pf_basic::string::safecopy(key_, key.c_str(), sizeof(key_) - 1);
   }
   const char *get_key() { return key_; }
   void set_result(int8_t result) { result_ = result; }
   int8_t get_result() const { return result_; }
   void set_columns(const std::string &columns) { columns_ = columns; }
   void set_rows(const std::string &rows) { rows_ = rows; }

 private:
   uint32_t get_data_size() const;

 private:
   uint16_t id_;
   int8_t result_;
   int8_t operate_;
   int32_t data_size_;
   char key_[128];
   std::string columns_;
   std::string rows_;

};

class DBResultFactory : public pf_net::packet::Factory {

 public:
   DBResultFactory() : id_{0} {}
   virtual ~DBResultFactory() {}

 public:
   void set_id(uint16_t id) { id_ = id; }
   
 public:
   virtual pf_net::packet::Interface *packet_create() {
     return new DBResult();
   }
   uint16_t packet_id() const {
     return id_;
   }
   virtual uint32_t packet_max_size() const {
     return SQL_LENGTH_MAX;
   }

 private:
   uint16_t id_;

};


} //namespace packet

} //namespace pf_cache

#endif //PF_CACHE_PACKET_DB_RESULT_H_
