/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id singleton.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.it@gmail.com>
 * @date 2016/07/10 18:38
 * @uses Template class for creating single-instance global classes.
 */
#ifndef PF_BASIC_SINGLETON_H_
#define PF_BASIC_SINGLETON_H_

#include "pf/basic/config.h"
#include "pf/sys/assert.h"

namespace pf_basic {

template <typename T> class Singleton {
 
 public:
   Singleton() {
     Assert(!singleton_);
#if defined(_MSC_VER) && _MSC_VER < 1200
     int offset = (int)(T*)1 - (int)(Singleton <T>*)(T*)1;
     singleton_ = (T*)((int)this + offset);
#else
     singleton_ = static_cast<T *>(this);
#endif
   }

   ~Singleton() {
     Assert(singleton_);
     singleton_ = nullptr;
   }

   static T &getsingleton() {
     Assert(singleton_);
     return *singleton_;
   }

   static T *getsingleton_pointer() {
     return singleton_;
   }

 protected:
   static T *singleton_;

 private:
   Singleton(const Singleton<T> &);
   Singleton& operator=(const Singleton<T> &);

};

}; //namespace pf_basic

#endif //PF_BASIC_SINGLETON_H_
