/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id interface.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/01/10 15:14
 * @uses The script module interface class.
*/
#ifndef PF_SCRIPT_INTERFACE_H_
#define PF_SCRIPT_INTERFACE_H_

#include "pf/script/config.h"
#include "pf/basic/type/variable.h"
#include "pf/basic/task_queue.tcc"

namespace pf_script {

class PF_API Interface {

 public:
   Interface() {};
   virtual ~Interface() { std::cout << "pf_script::~Interface" << std::endl; };

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
                          const pf_basic::type::variable_t &var) = 0;
   virtual void getglobal(const std::string &name, 
                          pf_basic::type::variable_t &var) = 0;
   virtual bool exists(const std::string &name) = 0;
   virtual bool call(const std::string &str) = 0;
   virtual bool call(const std::string &name, 
                     const pf_basic::type::variable_array_t &params,
                     pf_basic::type::variable_array_t &results) = 0;
   virtual void gccheck(int32_t) {};

 public:
   pf_basic::TaskQueue *task_queue() { return &task_queue_; }

 private:
   //Task queue can safe call in diffrent thead for script.
   pf_basic::TaskQueue task_queue_;

};

}; //namespace pf_script

#endif //PF_SCRIPT_INTERFACE_H_
