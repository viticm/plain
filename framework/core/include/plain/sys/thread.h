/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id thread.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/05/07 20:07
 * @uses The thread module.
 *     Pool refer: https://github.com/progschj/ThreadPool.
*/
#ifndef PLAIN_SYS_THREAD_H_
#define PLAIN_SYS_THREAD_H_

#include "plain/sys/config.h"
#include "plain/basic/global.h"

namespace plain {

class PLAIN_API ThreadPool {
 public:
   ThreadPool(size_t);
   template<class F, class... Args>
   auto enqueue(F&& f, Args&&... args) 
   -> std::future<typename std::result_of<F(Args...)>::type>;
   ~ThreadPool();
 private:
   // need to keep track of threads so we can join them
   std::vector< std::thread > workers_;
   // the task queue
   std::queue< std::function<void()> > tasks_;
   // synchronization
   std::mutex queue_mutex_;
   std::condition_variable condition_;
   bool stop_;
};
 

// 用于需要回收的线程数量统计，只要线程任务没有完成，主线程就会等待
class PLAIN_API ThreadCollect {

 public:
   ThreadCollect();
   ~ThreadCollect();

 public:
   static int32_t count() const {
     return count_;
   }

 private:
   static std::atomic<int32_t> count_;

};

} //namespace plain

#include "plain/sys/thread.tcc"

#endif //PLAIN_SYS_THREAD_H_
