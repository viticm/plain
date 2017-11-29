#include "pf/cache/db_store.h"
#include "pf/db/query.h"
#include "pf/engine/kernel.h"
#include "pf/net/connection/basic.h"
#include "pf/cache/packet/db_result.h"
#include "pf/cache/packet/db_query.h"

using namespace pf_cache::packet;

bool DBQuery::read(pf_net::stream::Input &istream) {
  type_ = istream.read_int8();
  operate_ = istream.read_int8();
  istream.read_string(key_, sizeof(key_) - 1);
  istream.read_string(sql_str_, sizeof(sql_str_) - 1);
  return true;
}

bool DBQuery::write(pf_net::stream::Output &ostream) {
  ostream << type_ << operate_ << key_ << sql_str_;
  return true;
}

uint32_t DBQuery::size() const {
  size_t result = 0;
  result += sizeof(type_);
  result += sizeof(operate_);
  result += strlen(key_);
  result += strlen(sql_str_);
  return static_cast<uint32_t>(result);
}

uint32_t DBQuery::execute(pf_net::connection::Basic *connection) {
  if (GLOBALS["cache.db.query_user_define"] == true) {
    return Interface::execute(connection);
  }
  if (!ENGINE_POINTER) return kPacketExecuteStatusContinue;
  auto dbenv = ENGINE_POINTER->get_db();
  if (!dbenv) return kPacketExecuteStatusContinue;
  DBResult packet;
  packet.set_result(DBResult::kResultFailed);
  pf_db::Query query;
  if (query.init(dbenv)) {
    query.set_sql(get_sql_str());
    if (query.query()) {
      packet.set_result(DBResult::kResultSuccess);
      if (kQuerySelect == get_type()) {
        char columns[CACHE_DB_TABLE_COLUMNS_SIZE]{0};
        char rows[100 * 1024]{0};
        query.fetch(columns, sizeof(columns), rows, sizeof(rows));
        packet.set_columns(columns);
        packet.set_rows(rows);
      }
    }
  }
  connection->send(&packet);
  return kPacketExecuteStatusContinue;
}

uint32_t DBQueryFactory::packet_max_size() const {
  uint32_t result = 0;
  result += sizeof(int8_t);
  result += sizeof(int8_t);
  result += 128;
  result += SQL_LENGTH_MAX;
  return result;
}
