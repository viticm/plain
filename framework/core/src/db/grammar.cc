#include "pf/support/helpers.h"
#include "pf/db/grammar.h"

using namespace pf_db;
using namespace pf_support;

//Convert an array of column names into a delimited string.
std::string Grammar::columnize(const std::vector<std::string> &columns) {
  std::vector<std::string> temp;
  for (const std::string &value : columns)
    temp.push_back(wrap(value));
  return implode(", ", temp);
}

//Create query parameter place-holders for an array.
std::string Grammar::parameterize(variable_set_t &values) {
  std::vector<std::string> temp;
  for (auto it = values.begin(); it != values.end(); ++it)
    temp.push_back(parameter(it->second));
  return implode(", ", temp);
}
