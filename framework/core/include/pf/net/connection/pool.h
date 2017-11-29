/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id pool.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.it@gmail.com>
 * @date 2014/06/23 14:10
 * @uses net connection pool class
 */
#ifndef PF_NET_CONNECTION_POOL_H_
#define PF_NET_CONNECTION_POOL_H_

#include "pf/net/connection/config.h"
#include "pf/sys/thread.h"
#include "pf/net/connection/basic.h"

namespace pf_net {

namespace connection {

class PF_API Pool {

 public:
   Pool();
   ~Pool();

 public:
   bool init(uint32_t maxcount = NET_CONNECTION_POOL_SIZE_DEFAULT);
   Basic *get(int16_t id);
   Basic *create(bool clear = true); //new
   bool init_data(uint16_t index, Basic *connection);
   void remove(int16_t id); //delete
   void lock();
   void unlock();
   uint32_t get_max_size() const { return max_size_; }
   uint32_t size() const { return size_; }
   bool create_default_connections();
   bool full() const { return size_ == max_size_; };

 private:
   //Basic **connections_; //注意，这是一个指向Base对象的数组指针
   std::vector< std::unique_ptr< Basic > > connections_;
   bool ready_;
   uint32_t position_;
   std::mutex mutex_;
   uint32_t size_;
   uint32_t max_size_;

};

}; //namespace connection

}; //namespace pf_net

//extern pf_net::connection::Pool* g_connectionpool;

#endif //PF_NET_CONNECTION_POOL_H_
