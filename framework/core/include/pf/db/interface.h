/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id interface.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/07/04 15:32
 * @uses the db manager class
 */
#ifndef PF_DB_INTERFACE_H_
#define PF_DB_INTERFACE_H_

#include "pf/db/config.h"

namespace pf_db {

class PF_API Interface {

 public:
   Interface() : isready_{false} {};
   virtual ~Interface() {};

 public:
   virtual bool init() = 0;
   virtual void select_db(const std::string &) {}
   virtual bool query(const std::string &sql_str) = 0;
   virtual bool fetch(int32_t orientation = 1, int32_t offset = 0) = 0;
   virtual int32_t get_affectcount() const = 0;
   virtual bool check_db_connect() = 0;
   bool isready() const { return isready_; };
   virtual bool getresult() const = 0;
   virtual int32_t get_columncount() const = 0;
   virtual const char *get_columnname(int32_t column_index) const = 0;
   void set_name(const std::string &name) { name_ = name; };
   void set_username(const std::string &username) { 
     username_ = username; 
   };
   void set_password(const std::string &password) {
     password_ = password;
   };

 public:
   virtual float get_float(int32_t column_index, int32_t &error_code) = 0;
   virtual int64_t get_int64(int32_t column_index, int32_t &error_code) = 0;
   virtual uint64_t get_uint64(int32_t column_index, int32_t &error_code) = 0;
   virtual int32_t get_int32(int32_t column_index, int32_t &error_code) = 0;
   virtual uint32_t get_uint32(int32_t column_index, int32_t &error_code) = 0;
   virtual int16_t get_int16(int32_t column_index, int32_t &error_code) = 0;
   virtual uint16_t get_uint16(int32_t column_index, int32_t &error_code) = 0;
   virtual int8_t get_int8(int32_t column_index, int32_t &error_code) = 0;
   virtual uint8_t get_uint8(int32_t column_index, int32_t &error_code) = 0;
   virtual int32_t get_string(int32_t column_index, 
                              char *buffer, 
                              int32_t buffer_length, 
                              int32_t &error_code) = 0;
   virtual int32_t get_field(int32_t column_index, 
                             char *buffer, 
                             int32_t buffer_length, 
                             int32_t &error_code) = 0;
   virtual int32_t get_binary(int32_t column_index, 
                              char *buffer, 
                              int32_t buffer_length, 
                              int32_t &error_code) = 0;
   virtual int32_t get_binary_withdecompress(int32_t column_index, 
                                             char *buffer, 
                                             int32_t buffer_length, 
                                             int32_t &error_code) = 0;
   virtual const char *get_data(
       int32_t column_index, const char *_default) const = 0;
   virtual db_columntype_t gettype(int32_t column_index) = 0;

 public:
   std::mutex *get_mutex() { return &mutex_; }

 protected:
   bool isready_;
   std::mutex mutex_;
   std::string name_;
   std::string username_;
   std::string password_;

};

}; //namespace pf_db

/* Lock db manager in mutli threads, p is manager pointer, n is lock name. */
#ifndef db_lock
#define db_lock(p,n) std::unique_lock<std::mutex> n(*(p)->get_mutex())
#endif

#endif //PF_DB_INTERFACE_H_
