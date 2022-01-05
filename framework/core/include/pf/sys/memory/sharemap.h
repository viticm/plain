/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id sharemap.h
 * @link https://github.com/viticm/plainframework1 for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm@126.com/viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm@126.com/viticm.ti@gmail.com>
 * @date 2017/05/17 12:07
 * @uses share memory hash map
 *       map implement:
 *         memory struct: [header|buckets|nodes] -> in map pool
 *                        * buckets is single linked list <share::map_bucket_t> 
 *                        * nodes is double linked list <share::map_node_t>
 *
 *         new node doing: key->hash->bucketindex->add bucket linked list first
 *                         * bucket linked list cur is the node in pool index.
 *
 *         find hash doing: key->hash->bucketindex-> find the node index
 *                          * see function getref(the way from new node)
 *         
 *         remove hash doing: Get the pool index from find hash,
 *                            * when index is invalid(not the INDEX_INVALID)
 *                              then find node and delete it in node list.
 *
 *         all node doing phrase means:
 *           key -> the map key(get/set first param)
 *           hash -> is the only number from key string(see function hashkey)
 *           bucketindex -> is the number from hash(see function bucketindex)
 *
 */
#ifndef PF_SYS_MEMORY_SHAREMAP_H_
#define PF_SYS_MEMORY_SHAREMAP_H_

#include "pf/sys/memory/config.h"
#include "pf/sys/memory/share.h"

namespace pf_sys {

namespace memory {

namespace share {


struct PF_API _map_node_struct {
  uint32_t hash;
  int32_t prev; //Prev index.
  int32_t next; //Node index.
  _map_node_struct();
  void clear();
};

struct PF_API map_bucket_struct {
  int32_t cur;
  map_bucket_struct();
  void clear();
};

using _map_node_t = struct _map_node_struct;
using map_bucket_t = struct map_bucket_struct;
using map_node_t = data_template<_map_node_t>;

//Map 正向迭代器
class PF_API map_iterator {

 public:
   
   //First value, -> will get key value.
   char *first;

   //Second value, -> will get data value.
   char *second;

 public:  
   map_iterator() :
    first{nullptr},
    second{nullptr},
    current_{-1},
    map_{nullptr} {
   }

   map_iterator(const map_iterator &obj) {
     this->first = obj.first;
     this->second = obj.second;
     this->current_ = obj.current_;
     this->map_ = obj.map_;
   }

   map_iterator(Map *_map, int32_t current) : 
     current_{current},
     map_{_map} {
     generate_data();
   }

   ~map_iterator() {}

 public:
   map_iterator &operator ++() {
     ++current_;
     generate_data();
     return *this;
   }
   
   map_iterator operator ++(int32_t) {
     map_iterator temp = *this;
     ++current_;
     generate_data();
     return temp;
   }

   map_iterator &operator --() {
     --current_;
     generate_data();
     return *this;
   }
   
   map_iterator operator --(int) {
     map_iterator temp = *this;
     --current_;
     generate_data();
     return temp;
   }

   map_iterator &operator +(int32_t n) {
     current_ += n;
     generate_data();
     return *this;
   }

   map_iterator operator +(int32_t n) const {
     return map_iterator(map_, current_ + n);
   }

   map_iterator &operator -(int32_t n) {
     current_ -= n;
     generate_data();
     return *this;
   }

   map_iterator operator -(int32_t n) const {
     return map_iterator(map_, current_ - n);
   }
   
   map_iterator &operator = (const map_iterator &o) {
     first = o.first;
     second = o.second;
     current_ = o.current_;
     map_ = o.map_;
     return *this;
   }

 public:
   int32_t base() const { return current_; }
   Map *map() const { return map_; }

 private:
   
   int32_t current_; //Current pool index.

   Map *map_; //Share map pointer.

 private:
   map_iterator operator = (const char *);
   
   void generate_data();

};

//Map 反向迭代器
class PF_API map_reverse_iterator {

 public:
   
   //First value, -> will get key value.
   char *first;

   //Second value, -> will get data value.
   char *second;

 public:  
   map_reverse_iterator() :
    first{nullptr},
    second{nullptr},
    current_{-1},
    map_{nullptr} {
   }

   map_reverse_iterator(const map_reverse_iterator &obj) {
     this->first = obj.first;
     this->second = obj.second;
     this->current_ = obj.current_;
     this->map_ = obj.map_;
   }

   map_reverse_iterator(Map *_map, int32_t current) : 
     current_{current},
     map_{_map} {
     generate_data();
   }

   ~map_reverse_iterator() {}

 public:
   map_reverse_iterator &operator ++() {
     --current_;
     generate_data();
     return *this;
   }
   
   map_reverse_iterator operator ++(int32_t) {
     map_reverse_iterator temp = *this;
     --current_;
     generate_data();
     return temp;
   }

   map_reverse_iterator &operator --() {
     ++current_;
     generate_data();
     return *this;
   }
   
   map_reverse_iterator operator --(int32_t) {
     map_reverse_iterator temp = *this;
     ++current_;
     generate_data();
     return temp;
   }

   map_reverse_iterator &operator +(int32_t n) {
     current_ -= n;
     generate_data();
     return *this;
   }

   map_reverse_iterator operator +(int32_t n) const {
     return map_reverse_iterator(map_, current_ - n);
   }

   map_reverse_iterator &operator -(int32_t n) {
     current_ += n;
     generate_data();
     return *this;
   }

   map_reverse_iterator operator -(int32_t n) const {
     return map_reverse_iterator(map_, current_ + n);
   }
   
   map_reverse_iterator &operator = (const map_reverse_iterator &o) {
     first = o.first;
     second = o.second;
     current_ = o.current_;
     map_ = o.map_;
     return *this;
   }

 public:
   int32_t base() const { return current_; }
   Map *map() const { return map_; }

 private:
   
   int32_t current_; //Current pool index.

   Map *map_; //Share map pointer.

 private:
   map_iterator operator = (const char *);
   
   void generate_data();

};

inline bool operator == (const map_iterator &x, const map_iterator &y) {
  return x.base() == y.base() && x.map() == y.map();
}

inline bool operator != (const map_iterator &x, const map_iterator &y) {
  return !(x == y);
}

inline bool operator == 
  (const map_reverse_iterator &x, const map_reverse_iterator &y) {
  return x.base() == y.base() && x.map() == y.map();
}

inline bool operator != 
  (const map_reverse_iterator &x, const map_reverse_iterator &y) {
  return !(x == y);
}

class PF_API MapPool : public UnitPool<map_node_t> {

 public:
   MapPool();
   ~MapPool();

 public:
   bool init(uint32_t key, size_t size, size_t datasize, bool create);

 public:
   map_node_t *new_obj();
   void delete_obj(map_node_t *obj);
   char *getbuckets();
   uint32_t bucketindex(uint32_t hash);
   uint32_t hashkey(const char *str);

 private:
   char *getdata(uint32_t size, uint32_t index);

};

class PF_API Map {

 public:
   Map();
   ~Map();

 public:
   bool init(uint32_t key, 
             size_t size, 
             size_t keysize, 
             size_t valuesize,
             bool create = false);
   void clear();
   const char *get(const char *key);
   bool set(const char *key, const char *value);
   void remove(const char *key);
   MapPool *getpool() { return pool_.get(); };

 public: //to standard map functions.
   inline const char *operator[](const char *key) {
     return get(key);
   }
   inline void erase(const char *key) {
     remove(key);
   }
   inline size_t size() const {
     return pool_->get_position();
   }
   inline size_t full() const {
     return pool_->full();
   }

 public:
   size_t key_size() const { return keysize_; }
   size_t value_size() const { return valuesize_; }

 public:
   inline map_iterator begin() {
     return map_iterator(this, 0);
   }

   inline map_iterator end() {
     return map_iterator(this, pool_->get_position());
   }

   inline map_reverse_iterator rbegin() {
     return map_reverse_iterator(this, pool_->get_position() - 1);
   }

   inline map_reverse_iterator rend() {
     return map_reverse_iterator(this, -1);
   }

   inline map_iterator find(const char *key) {
     auto index = getref(key);
     return map_iterator(this, 
                         ID_INVALID == index ? pool_->get_position() : index);
   }

   void pop_front(std::string &key, std::string &var) {
     auto it = begin();
     key = it.first; var = it.second;
     erase(it.first);
   }
   void pop_back(std::string &key, std::string &var) {
     auto it = rbegin();
     key = it.first; var = it.second;
     erase(it.first);
   }

   inline bool is_ready() const { return ready_; }

 private:
   size_t keysize_;
   size_t valuesize_;
   std::unique_ptr<MapPool> pool_;
   map_bucket_t *buckets_;
   bool ready_;

 private:
   map_node_t *newnode(const char *key, const char *value);
   void addnode(map_node_t *node);
   int32_t getref(const char *key);

};

} //namespace share

} //namespace memory

} //namespace memory

#endif //PF_SYS_MEMORY_SHAREMAP_H_
