/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id assistant.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm@126.com/viticm.ti@gmail.com>
 * @date 2015/01/25 20:38
 * @uses the util compressor assistant class
*/
#ifndef PF_UTIL_COMPRESSOR_ASSISTANT_H_
#define PF_UTIL_COMPRESSOR_ASSISTANT_H_

#include "pf/util/compressor/config.h"

namespace pf_util {

namespace compressor {

class PF_API Assistant {

 public:
   Assistant();
   ~Assistant();

 public:
   void set_workmemory(void *memory) { workmemory_ = memory; };
   void *get_workmemory() { return workmemory_; };
   bool isenable() const { return isenable_; };
   void enable(bool enable, uint64_t threadid = 0);
   bool log_isenable() const { return log_isenable_; };
   void log_enable(bool _enable) { log_isenable_ = _enable; };
   void compressframe_inc() { ++compressframe_success_; };
   uint32_t get_compressframe() const { return compressframe_; };
   void compressframe_successinc() { ++compressframe_success_; };
   uint32_t get_success_compressframe() const { 
     return compressframe_success_; 
   };

 private:
   void *workmemory_;
   bool isenable_;
   bool log_isenable_;
   uint32_t compressframe_;
   uint32_t compressframe_success_;

};

} //namespace compressor

} //namespace pf_util

#endif //PF_UTIL_COMPRESSOR_ASSISTANT_H_
