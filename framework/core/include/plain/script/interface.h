/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id interface.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2023/04/01 16:50
 * @uses The script module interface class.
*/
#ifndef PLAIN_SCRIPT_INTERFACE_H_
#define PLAIN_SCRIPT_INTERFACE_H_

#include "plain/script/config.h"
#include "plain/basic/type/variable.h"
#include "plain/basic/task_queue.tcc"

namespace plain::script {

class PLAIN_API Interface {

 public:
   Interface() {};
   virtual ~Interface() {};

 public:
   virtual bool init() = 0;
   virtual bool bootstrap(const std::string &filename = "") = 0;
   virtual void release() = 0;

 public:
   virtual bool load(const std::string &filename) = 0;
   virtual bool reload(const std::string &filename) = 0;

 public: //env.
   virtual void set_rootpath(const std::string &path) = 0;
   virtual void set_workpath(const std::string &path) = 0;

 public:
   virtual void register_function(const std::string &name, void *pointer) = 0;
   virtual void setglobal(const std::string &name, 
                          const plain::variable_t &var) = 0;
   virtual void getglobal(const std::string &name, 
                          variable_t &var) = 0;
   virtual bool exists(const std::string &name) = 0;
   virtual bool call(const std::string &str) = 0;
   virtual bool call(const std::string &name, 
                     const variable_array_t &params,
                     variable_array_t &results) = 0;
   virtual void gccheck(int32_t) {};

 public:
   TaskQueue *task_queue() { return &task_queue_; }

 private:
   
   // Task queue can safe call in diffrent thead for script.
   TaskQueue task_queue_;

};

} // namespace plain::script

#endif // PLAIN_SCRIPT_INTERFACE_H_
