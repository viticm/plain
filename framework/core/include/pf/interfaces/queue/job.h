/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id job.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/07/27 17:09
 * @uses The queue job interface class.
 */

#ifndef PF_INTERFACES_QUEUE_JOB_H_
#define PF_INTERFACES_QUEUE_JOB_H_

#include "pf/interfaces/queue/config.h"

namespace pf_interfaces {

namespace queue {

class PF_API Job {

 public:
   Job() {}
   virtual ~Job() {}

 public:

   // Get the UUID of the job.
   virtual std::string uuid() const = 0;

   // Get the job identifier.
   virtual std::string get_id() const = 0;

   // Get the decoded body of the job.
   virtual std::vector<std::string> payload() const = 0;

   // Fire the job.
   virtual void fire() = 0;

   // Release the job back into the queue. 
   //
   // Accepts a delay specified in seconds. 
   virtual void release(int32_t delay = 0) = 0;

   // Determine if the job was released back into the queue. 
   virtual bool is_released() const = 0;

   // Remove the job from the queue. 
   virtual void remove() = 0;

   // Determine if the job has been removed.
   virtual bool is_removed() const = 0;

   // Determine if the job has been removed or released.
   virtual bool is_removed_or_released() const = 0;

   // Get the number of times the job has been attempted. 
   virtual int32_t attempts() const = 0;

   // Determine if the job has been marked as a failure. 
   virtual bool has_failed() const = 0;

   // Mark the job as "failed".
   virtual void mark_as_failed() = 0;

   // Remove the job, call the "failed" method, and raise the failed job event.
   virtual void fail(std::exception *e = nullptr) = 0;

   // Get the number of times to attempt a job.
   virtual int32_t max_tries() const = 0;

   // Get the maximum number of exceptions allowed, regardless of attempts.
   virtual int32_t max_exceptions() const = 0;

   // Get the number of seconds the job can run.
   virtual int32_t timeout() const = 0;

   // Get the timestamp indicating when the job should timeout.
   virtual int32_t retry_until() const = 0;

   // Get the name of the queued job class.
   virtual std::string name() const = 0;

   // Get the resolved name of the queued job class.
   //
   // Resolves the name of "wrapped" jobs such as class-based handlers. 
   virtual std::string resolve_name() = 0;

   // Get the name of the connection the job belongs to.
   virtual std::string get_connection_name() const = 0;

   // Get the name of the queue the job belongs to. 
   virtual std::string get_queue() const = 0;

   // Get the raw body string for the job.
   virtual std::string get_raw_body() const = 0;

};

} // namespace queue

} // namespace pf_interfaces

#endif // PF_INTERFACES_QUEUE_JOB_H_
