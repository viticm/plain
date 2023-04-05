/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 * $Id singleton.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.it@gmail.com>
 * @date 2023/03/31 20:14
 * @uses Template class for creating single-instance global classes.
 */
#ifndef PLAIN_BASIC_SINGLETON_H_
#define PLAIN_BASIC_SINGLETON_H_

#include "plain/basic/config.h"

namespace plain {

template <typename T>
class PLAIN_API Singleton : noncopyable {
 
 public:
   Singleton() = default;

   ~Singleton() = default;

   static std::shared_ptr<T> get_instance() {
     std::call_once(once_flag_, [&]{
       singleton_ = std::make_shared<T>();
     });
     return singleton_;
   }

 protected:
   static std::shared_ptr<T> singleton_;

 private:
   static std::once_flag once_flag_;

};

} // namespace plain

#define PLAIN_SINGLETON_DECL(T) \
template <> \
std::shared_ptr<T> plain::Singleton<T>::singleton_{nullptr}; \
template <> \
std::once_flag plain::Singleton<T>::once_flag_{};

#endif //PLAIN_BASIC_SINGLETON_H_
