#include "pf/engine/kernel.h"
#include "pf/cache/repository.h"
#include "pf/cache/db_store.h"
#include "pf/cache/packet/db_result.h"

using namespace pf_cache::packet;

bool DBResult::read(pf_net::stream::Input &istream) {
  result_ = istream.read_int8();
  operate_ = istream.read_int8();
  istream.read_string(key_, sizeof(key_) - 1);
  istream >> columns_;
  istream >> rows_;
  data_size_ = static_cast<int32_t>(columns_.size() + rows_.size());
  return true;
}

bool DBResult::write(pf_net::stream::Output &ostream) {
  ostream << result_ << operate_ << key_ << columns_ << rows_;
  data_size_ = static_cast<int32_t>(columns_.size() + rows_.size());
  return true;
}

uint32_t DBResult::size() const {
  uint32_t result = 0;
  result += sizeof(result_);
  result += sizeof(operate_);
  result += 128;
  result += data_size_;
  return result;
}

uint32_t DBResult::execute(pf_net::connection::Basic *) {
  if (!ENGINE_POINTER) return kPacketExecuteStatusContinue;
  auto cache_manager = ENGINE_POINTER->get_cache();
  if (!cache_manager) return kPacketExecuteStatusContinue;
  auto store = 
    dynamic_cast< pf_cache::DBStore *>(cache_manager->get_db_dirver()->store());
  store->set(get_key(), columns_, rows_); 
  return kPacketExecuteStatusContinue;
}

uint32_t DBResult::get_data_size() const {
  return static_cast<uint32_t>(columns_.size() + rows_.size());
}
