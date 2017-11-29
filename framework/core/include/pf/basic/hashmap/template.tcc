/**
 * PAP Engine ( https://github.com/viticm/pap )
 * $Id template.tcc
 * @link https://github.com/viticm/pap for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm@126.com )
 * @license
 * @user viticm<viticm@126.com>
 * @date 2016/05/06 23:36
 * @uses the hash map template implement
 */
#ifndef PF_BASIC_HASHMAP_TEMPLATE_TCC_
#define PF_BASIC_HASHMAP_TEMPLATE_TCC_

#include "pf/basic/hashmap/template.h"

namespace pf_basic {

namespace hashmap {

template <class T_key, class T_value>
Template<T_key, T_value>::Template() {
   maxcount_ = 0;
}

template <class T_key, class T_value> 
Template<T_key, T_value>::~Template() {
  maxcount_ = 0;
}

template <class T_key, class T_value> 
Template<T_key, T_value>::Template(const Template &object) {
  clear();
  if (object.getcount() > 0) {
    init(object.getcount());
    hashmap_ = object.hashmap_;
  }
}

template <class T_key, class T_value> 
Template<T_key, T_value>::Template(const Template *object) {
  clear();
  if (object && object->getcount() > 0) {
    init(object->getcount());
    hashmap_ = object->hashmap_;
  }
}

template <class T_key, class T_value> 
Template<T_key, T_value> &
  Template<T_key, T_value>::operator = (const Template &object) {
  clear();
  if (object.getcount() > 0) {
    init(object.getcount());
    hashmap_ = object.hashmap_;
  }
  return *this;
}

template <class T_key, class T_value> 
Template<T_key, T_value> *
  Template<T_key, T_value>::operator = (const Template *object) {
  clear();
  if (object && object->getcount() > 0) {
    init(object->getcount());
    hashmap_ = object->hashmap_;
  }
  return this;
}

template <class T_key, class T_value> 
void Template<T_key, T_value>::init(uint32_t count) {
  clear();
  maxcount_ = count;
}

template <class T_key, class T_value> 
bool Template<T_key, T_value>::add(T_key key, T_value value) {
  if (hashmap_.size() >= maxcount_) return false;
  hashmap_.insert(typename hashmap_t::value_type(key, value));
  return true;
}

template <class T_key, class T_value> 
bool Template<T_key, T_value>::set(T_key key, T_value value) {
  iterator_t iterator = hashmap_.find(key);
  if (iterator != hashmap_.end()) {
    iterator->second = value;
    return true;
  }
  return false;
}

template <class T_key, class T_value> 
T_value Template<T_key, T_value>::get(T_key key) {
  iterator_t iterator = hashmap_.find(key);
  if (iterator != hashmap_.end()) return iterator->second;
  return 0;
}

template <class T_key, class T_value> 
bool Template<T_key, T_value>::isfind(T_key key) {
  iterator_t iterator = hashmap_.find(key);
  if (iterator != hashmap_.end()) return true;
  return false;
}

template <class T_key, class T_value> 
bool Template<T_key, T_value>::remove(T_key key) {
  iterator_t iterator = hashmap_.find(key);
  if (iterator != hashmap_.end()) {
    hashmap_.erase(iterator);
    return true;
  }
  return false;
}

template <class T_key, class T_value> 
void Template<T_key, T_value>::clear() {
  hashmap_.clear();
}

template <class T_key, class T_value> 
uint32_t Template<T_key, T_value>::getcount() const {
  return static_cast<uint32_t>(hashmap_.size());
}

template <class T_key, class T_value> 
uint32_t Template<T_key, T_value>::get_maxcount() const {
  return maxcount_;
}

template <class T_key, class T_value> 
void Template<T_key, T_value>::set_maxcount(uint32_t count) {
  maxcount_ = count;
}

}; //namespace hashmap

}; //namespace pf_basic

#endif //PF_BASIC_HASHMAP_TEMPLATE_TCC_
