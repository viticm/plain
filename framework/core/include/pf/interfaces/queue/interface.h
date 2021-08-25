/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id interface.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/07/27 15:40
 * @uses The queue interface class.
 */

#ifndef PF_INTERFACES_QUEUE_INTERFACE_H_
#define PF_INTERFACES_QUEUE_INTERFACE_H_

#include "pf/interfaces/queue/config.h"

namespace pf_interfaces {

namespace queue {

class PF_API Interface {

 public:
   Interface() {}
   virtual ~Interface() {}

 public:

   // Get the size of the queue.
   virtual size_t size(const std::string &queue = "") = 0;

   // Push a new job onto the queue.
   virtual void push(const std::string &job,
                     const std::string &data = "",
                     const std::string &queue = "") = 0;

   // Push a new job onto the queue.
   virtual void push_on(const std::string &queue,
                        const std::string &job,
                        const std::string &data = "") = 0;

   // Push a raw payload onto the queue.
   virtual void push_raw(const std::string &payload,
                         const std::string &queue = "",
                         const std::vector<std::string> &options = {}) = 0;

   // Push a new job onto the queue after a delay.
   virtual void later(int32_t delay,
                      const std::string &job,
                      const std::string &data = "",
                      const std::string &queue = "") = 0;

   // Push a new job onto the queue after a delay.
   virtual void later_on(const std::string &queue,
                         int32_t delay,
                         const std::string &job,
                         const std::string &data = "") = 0;

   // Push an array of jobs onto the queue.
   virtual void bulk(const std::vector<std::string> &jobs,
                     const std::string &data = "",
                     const std::string &queue = "") = 0;

   // Pop the next job off of the queue.
   virtual void pop(queue::Job *job = nullptr) = 0;

   // Get the connection name for the queue. 
   virtual std::string get_connection_name() const = 0;

   // Set the connection name for the queue. 
   virtual Interface &set_connection_name(const std::string &name) = 0;

};

} // namespace queue

} // namespace pf_interfaces

#endif // PF_INTERFACES_QUEUE_INTERFACE_H_
