#include "pf/db/interface.h"
#include "pf/basic/logger.h"
#include "pf/db/factory.h"

using namespace pf_db;

eid_t Factory::newenv(const config_t &config) {
  eid_t eid = neweid();
  if (DB_EID_INVALID == eid) return eid;
  auto func_envcreator = get_env_creator_db(config.type);
  if (is_null(func_envcreator)) {
    last_del_eid_ = eid;
    return DB_EID_INVALID;
  }
  Interface *env = func_envcreator();
  env->set_name(config.name);
  env->set_username(config.username);
  env->set_password(config.password);
  std::unique_ptr< Interface > pointer(env);
  envs_[eid] = std::move(pointer);
  return eid;
}

eid_t Factory::newenv(Interface *env) {
  eid_t eid = neweid();
  if (DB_EID_INVALID == eid) return eid;
  std::unique_ptr< Interface > pointer(env);
  envs_[eid] = std::move(pointer);
  return eid;
}

eid_t Factory::neweid() {
  std::unique_lock<std::mutex> autolock(mutex_);
  eid_t eid = DB_EID_INVALID;
  eid = DB_EID_INVALID == last_del_eid_ ?
        static_cast<eid_t>(envs_.size() + 1) : last_del_eid_;
  last_del_eid_ = DB_EID_INVALID;
  return eid;
}
