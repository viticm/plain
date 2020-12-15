/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id null.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2018/11/09 15:59
 * @uses the db manager null class
 */
#ifndef PF_DB_NULL_H_
#define PF_DB_NULL_H_

#include "pf/db/interface.h"

namespace pf_db {

class PF_API Null : public Interface {

 public:
   Null() {};
   virtual ~Null() {};

 public:
   virtual bool init() { return true; };
   virtual void select_db(const std::string &) {}
   virtual bool query(const std::string &) { return false; };
   virtual bool fetch(int32_t, int32_t) { return false; };
   virtual int32_t get_affectcount() const { return 0; };
   virtual bool check_db_connect(bool directly = false) { 
    UNUSED(directly); 
    return true; 
   };
   bool isready() const { return isready_; };
   virtual bool getresult() const { return false; };
   virtual int32_t get_columncount() const { return 0; };
   virtual const char *get_columnname(int32_t) const { return ""; };

 public:
   virtual float get_float(int32_t, int32_t &) { return 0; };
   virtual int64_t get_int64(int32_t, int32_t &) { return 0; };
   virtual uint64_t get_uint64(int32_t, int32_t &) { return 0; };
   virtual int32_t get_int32(int32_t, int32_t &) { return 0; };
   virtual uint32_t get_uint32(int32_t, int32_t &) { return 0; };
   virtual int16_t get_int16(int32_t, int32_t &) { return 0; };
   virtual uint16_t get_uint16(int32_t, int32_t &) { return 0; };
   virtual int8_t get_int8(int32_t, int32_t &) { return 0; };
   virtual uint8_t get_uint8(int32_t, int32_t &) { return 0; };
   virtual int32_t get_string(int32_t, 
                              char *, 
                              int32_t, 
                              int32_t &) { return 0; };
   virtual int32_t get_field(int32_t, 
                             char *, 
                             int32_t, 
                             int32_t &) { return 0; };
   virtual int32_t get_binary(int32_t, 
                              char *, 
                              int32_t, 
                              int32_t &) { return 0; };
   virtual int32_t get_binary_withdecompress(int32_t, 
                                             char *, 
                                             int32_t, 
                                             int32_t &) { return 0; };
   virtual const char *get_data(
       int32_t, const char *) const { return 0; };
   virtual db_columntype_t gettype(int32_t) { return kDBColumnTypeString; };

};

} //namespace pf_db

#endif //PF_DB_NULL_H_
