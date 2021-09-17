#pragma once

#include <string>

struct T1{};

struct T2{};

struct Value {
  int value; // int value{-1}; Not active in c++11.
};

template <typename Event>
struct Tag {
  Event data;
  std::string tag;
};

struct WaitPerk {

};
