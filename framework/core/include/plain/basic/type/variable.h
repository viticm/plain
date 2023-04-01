/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id variable.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2023/03/31 18:08
 * @uses Base module the variable type(like script variables).
 *       Base on string, also can do string convert to diffrent type.
 *       Refer: php/lua or more script codes.
*/
#ifndef PLAIN_BASIC_TYPE_VARIABLE_H_
#define PLAIN_BASIC_TYPE_VARIABLE_H_

#include "plain/basic/type/config.h"

namespace plain {

template <typename T>
Variable std_convert_type(T);

struct PLAIN_API variable_struct {
  using enum Variable;
  Variable type;
  std::string data;
  mutable std::mutex mutex;
  variable_struct() : type{Invalid}, data{""} {}

  variable_struct(const variable_t &object); 
  variable_struct(const variable_t *object);
  variable_struct(const std::string &value);
  variable_struct(const char *value);
  template <typename T>
  variable_struct(T value);
  
  template <typename T>
  T get() const; 
  template <typename T>
  T _get() const; //Not safe in multi threads.
  const char *c_str() const;

  variable_t &operator = (const variable_t &object);
  variable_t *operator = (const variable_t *object);
  variable_t &operator = (const std::string &value);
  variable_t &operator = (const char *value);
  variable_t &operator = (char *value);
  template <typename T>
  variable_t &operator = (T value);

  variable_t &operator += (const variable_t &object);
  variable_t *operator += (const variable_t *object);
  variable_t &operator += (const std::string &value);
  variable_t &operator += (const char *value);
  template <typename T>
  variable_t &operator += (T value);

  variable_t &operator -= (const variable_t &object);
  variable_t *operator -= (const variable_t *object);
  template <typename T>
  variable_t &operator -= (T value);

  variable_t &operator *= (const variable_t &object);
  variable_t *operator *= (const variable_t *object);
  template <typename T>
  variable_t &operator *= (T value);

  variable_t &operator /= (const variable_t &object);
  variable_t *operator /= (const variable_t *object);
  template <typename T>
  variable_t &operator /= (T value);

  variable_t &operator ++ ();
  variable_t &operator -- ();
  variable_t &operator ++ (int32_t);
  variable_t &operator -- (int32_t);
  
  bool operator == (const variable_t &object) const;
  bool operator == (const variable_t *object) const;
  bool operator == (const std::string &value) const;
  bool operator == (const char *value) const;
  template <typename T>
  bool operator == (T value) const;
  
  bool operator != (const variable_t &object) const;
  bool operator != (const variable_t *object) const;
  bool operator != (const std::string &value) const;
  bool operator != (const char *value) const;
  template <typename T>
  bool operator != (T value) const;

  bool operator < (const variable_t &object) const; //for map
  bool operator < (const variable_t *object) const;
  bool operator < (const std::string &value) const;
  bool operator < (const char *value) const;
  template <typename T>
  bool operator < (T value) const;

  bool operator > (const variable_t &object) const; //for map
  bool operator > (const variable_t *object) const;
  bool operator > (const std::string &value) const;
  bool operator > (const char *value) const;
  template <typename T>
  bool operator > (T value) const;

  operator const std::string();
  operator const char *();
  template <typename T>
  operator T();

}; //PLAIN变量，类似脚本变量

} //namespace plain

#include "plain/basic/type/variable.tcc"

#endif //PLAIN_BASIC_TYPE_VARIABLE_H_
