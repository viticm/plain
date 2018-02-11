#include <algorithm>
#include "pf/basic/string.h"
#include "pf/support/helpers.h"
#include "pf/db/grammar.h"

using namespace pf_db;
using namespace pf_support;
using namespace pf_basic::string;

//Wrap a value that has an alias.
std::string Grammar::wrap_aliased_value(
    const variable_t &value, bool prefix_alias) {
  auto segments = explode(" as ", value.data);
  std::cout << "wrap_aliased_value: " << segments.size() << std::endl;
  if (segments.size() != 2) return "";
  // If we are wrapping a table we need to prefix the alias with the table prefix
  // as well in order to generate proper syntax. If this is a column of course
  // no prefix is necessary. The condition will be true when from wrap_table.
  if (prefix_alias)
    segments[1] = table_prefix_ + segments[1].data;
  return wrap(segments[0].data) + " as " + wrap_value(segments[1].data);
}

//Wrap the given value segments.
std::string Grammar::wrap_segments(const variable_array_t &segments) {
  variable_array_t r;
  std::cout << "wrap_segments: " << segments.size() << std::endl;
  for (size_t i = 0; i < segments.size(); ++i) {
    std::cout << "wrap_segments xx: " << segments[i].data << "|" << std::endl;
    if (0 == i && segments.size() > 1)
      r.push_back(wrap_table(segments[i].data));
    else
      r.push_back(wrap_value(segments[i]));
  }
  return implode(".", r);
}

//Wrap a single string in keyword identifiers.
std::string Grammar::wrap_value(const variable_t &value) {
  if (value != "*")
    return "\"" + str_replace("\"", "\"\"", value.data) + "\"";
  return value.data;
}

//Wrap a value in keyword identifiers.
std::string Grammar::wrap(const variable_t &value, bool prefix_alias) {
  if (DB_EXPRESSION_TYPE == value.type) return value.data;
  // If the value being wrapped has a column alias we will need to separate out 
  // the pieces so we can wrap each of the segments of the expression on it 
  // own, and then joins them both back together with the "as" connector.
  std::string temp{value.data};
  std::transform(
      temp.begin(), temp.end(), temp.begin(), (int (*)(int))std::tolower);
  if (temp.find(" as ") != std::string::npos) 
    return wrap_aliased_value(temp, prefix_alias);
  std::cout << "wrap: " << value.data << "|" << std::endl;
  return wrap_segments(explode(".", value.data));
}

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
