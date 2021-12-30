#include "pf/basic/string.h"
#include "pf/basic/logger.h"
#include "pf/basic/io.tcc"
#include "pf/basic/stringstream.h"
#include "pf/basic/type/variable.h"
#include "pf/engine/kernel.h"
#include "pf/net/connection/basic.h"
#include "pf/script/factory.h"
#include "pf/script/interface.h"
#include "pf/net/packet/callscript.h"

using namespace pf_net::packet;
using namespace pf_basic;

bool CallScript::read(pf_net::stream::Input &istream) {
  istream.read_string(func_, sizeof(func_) - 1);
  char temp[102400]{0,};
  istream.read_string(temp, sizeof(temp) - 1);
  params_ = temp;
  istream >> eid_;
  return true;
}

bool CallScript::write(pf_net::stream::Output &ostream) {
  ostream << func_;
  ostream << params_;
  ostream << eid_;
  return true;
}

uint32_t CallScript::size() const {
  size_t result = 0;
  result += sizeof(uint32_t);
  result += strlen(func_);
  result += sizeof(uint32_t);
  result += params_.size();
  result += sizeof(eid_);
  return static_cast<uint32_t>(result);
}

void CallScript::set_params(const std::vector<std::string> &params) {
  int8_t _size = static_cast<int8_t>(params.size());
  if (0 == _size) return;
  char temp[102400]{0,};
  stringstream sstream(temp, sizeof(temp) - 1);
  sstream << _size;
  for (const std::string &item : params)
    sstream << item;
}

void CallScript::get_params(std::vector<std::string> &params) {
  auto _size = params_.size();
  if (0 == _size) return;
  char temp[102400]{0,};
  string::safecopy(temp, params_.c_str(), sizeof(temp) - 1);
  stringstream sstream(temp, _size);
  int8_t count{0}; 
  sstream >> count;
  if (count <= 0) return;
  params.clear();
  for (int8_t i = 0; i < count; ++i) {
    std::string item{""};
    sstream >> item;
    params.emplace_back(item);
  }
}

uint32_t CallScript::execute(pf_net::connection::Basic *connection) {
  UNUSED(connection);
  if (0 == strlen(func_)) return kPacketExecuteStatusError;
  auto script = ENGINE_POINTER->get_script();
  if (eid_ != -1) {
    auto factory = ENGINE_POINTER->get_script_factory();
    script = !is_null(factory) ? factory->getenv(eid_) : nullptr;
  }
  if (is_null(script)) return kPacketExecuteStatusError;
  std::vector<std::string> _params;
  get_params(_params);
  type::variable_array_t params;
  for (const std::string &item : _params)
    params.emplace_back(item);
  type::variable_array_t results;
  script->call(func_, params, results);
  return kPacketExecuteStatusContinue;
}
