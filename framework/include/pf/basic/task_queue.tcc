/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id task_queue.tcc
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/05/10 11:23
 * @uses The task queue for single thread.
*/
#ifndef PF_BASIC_TASK_QUEUE_TCC_
#define PF_BASIC_TASK_QUEUE_TCC_

#include "pf/basic/config.h"

namespace pf_basic {

class TaskQueue {

 public:
   TaskQueue() : stop_{false} {};
   ~TaskQueue() {
     {
       std::unique_lock<std::mutex> lock(queue_mutex_);
       stop_ = true;
     }
     work_all();
   };

 public:
   template<class F, class... Args>
   auto enqueue(F&& f, Args&&... args) 
     -> std::future<typename std::result_of<F(Args...)>::type> {
     using return_type = typename std::result_of<F(Args...)>::type;
     auto task = std::make_shared< std::packaged_task<return_type()> >(
         std::bind(std::forward<F>(f), std::forward<Args>(args)...)
       );
     std::future<return_type> res = task->get_future();
     {
       std::unique_lock<std::mutex> lock(queue_mutex_);

       //Don't allow enqueueing after stopping the TaskQueue
       if (stop_)
         throw std::runtime_error("enqueue on stopped TaskQueue");
       tasks_.emplace([task](){ (*task)(); });
     }
     return res;
   }

 public:
   void work_one() {
     std::function<void()> task;
     {
       std::unique_lock<std::mutex> lock(queue_mutex_);
       if (tasks_.empty()) return;
       task = std::move(tasks_.front());
       tasks_.pop();
     }
     task();
   };
   void work_all() {
     while (!tasks_.empty()) work_one();
   };

 private:
   //The task queue
   std::queue< std::function<void()> > tasks_;
   //Synchronization
   std::mutex queue_mutex_;
   //Stop flag
   bool stop_;

};

}; //namespace pf_basic

#endif //PF_BASIC_TASK_QUEUE_TCC_
