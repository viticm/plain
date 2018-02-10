/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id helpers.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/12/14 14:25
 * @uses your description
*/
#ifndef PF_SUPPORT_HELPERS_H_
#define PF_SUPPORT_HELPERS_H_

#include "pf/support/config.h"
#include "pf/support/collection.h"
#include "pf/basic/type/variable.h"

namespace pf_support {

//Create a collection from the given value.
template <typename T>
Collection<T> collect(const std::vector<T> &value) {
  Collection<T> o(value);
  return o;
}

//Concatenate values of a given key as a string.
inline std::string implode(const std::string &glue, 
                           const pf_basic::type::variable_array_t &array) {
  std::string r{""};
  auto it = array.begin();
  for (;it != array.end(); ++it) {
    r += it != array.begin() && it != array.end() ? 
         glue + (*it).data : (*it).data;
  }
  return r;
}

//Split a string by string.
inline pf_basic::type::variable_array_t explode(
    const std::string &delimiter, const std::string &str) {
  pf_basic::type::variable_array_t r;
  if ("" == str) return r;
  size_t last_found{0};
  for (;;) {
    auto found = 
      str.find(delimiter, 0 == last_found ? 0 : last_found + 1);
    if (found != std::string::npos) {
      auto pos = 0 == last_found ? 0 : last_found + 1; 
      r.push_back(str.substr(pos, found - last_found - 1));
      last_found = found;
    } else {
      r.push_back(str.substr(0 == last_found ? 0 : last_found + 1, str.size()));      
      break;
    }
  }
  return r;
}

//Concatenate values of a given key as a string.
inline std::string implode(const std::string &glue, 
                           const std::vector<std::string> &array) {
  std::string r{""};
  auto it = array.begin();
  for (;it != array.end(); ++it) {
    r += it != array.begin() && it != array.end() ? 
         glue + (*it) : (*it);
  }
  return r;
}

//Check the value is empty.
inline bool empty(const pf_basic::type::variable_t &value) {
  return pf_basic::type::kVariableTypeInvalid == value.type;
}

//Get the array keys of one variable set.
inline std::vector<std::string> array_keys(
    const pf_basic::type::variable_set_t &array) {
  std::vector<std::string> r;
  for (auto it = array.begin(); it != array.end(); ++it)
    r.push_back(it->first);
  return r;
}

//Get the array values of one variable set.
inline pf_basic::type::variable_array_t array_values(
    const pf_basic::type::variable_set_t &array) {
  pf_basic::type::variable_array_t r;
  for (auto it = array.begin(); it != array.end(); ++it)
    r.push_back(it->second);
  return r;
}

// Checks if a value exists in an array
template <typename T>
bool in_array(const T &needle, 
              const std::vector<T> &haystack) {
  for (const T &item : haystack) {
    if (needle == item) return true;
  }
  return false;
}

//Safe get vector array elements.
template <typename T>
T get(const std::vector<T> &array, size_t n) {
  T r;
  if (array.size() < n + 1) return r;
  r = array[n];
  return r;
}

//Filters elements of an array using a callback function.
template <typename T>
std::vector<T> array_filter(const std::vector<T> &array, 
                            std::function<bool(const T&)> callback) {
  std::vector<T> result;
  for (const T &item : array) {
    if (callback(item)) 
      result.push_back(item);
  }
  return result;
}

//Merge one or more arrays.
template <typename T_k, typename T_v>
std::map<T_k, T_v> array_merge(const std::map<T_k, T_v> &array1, 
                               const std::map<T_k, T_v> &array2) {
  std::map<T_k, T_v> result;
  for (auto it = array1.begin(); it != array1.end(); ++it)
    result[it->first] = it->second;
  for (auto it = array2.begin(); it != array2.end(); ++it)
    result[it->first] = it->second;
  return result;
}

}

#endif //PF_SUPPORT_HELPERS_H_
