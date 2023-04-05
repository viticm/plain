#include "plain/script/factory.h"
#include "plain/script/interface.h"

using namespace plain::script;

eid_t Factory::newenv(const config_t& config) {
  eid_t eid = neweid();
  if (SCRIPT_EID_INVALID == eid) return eid;
  auto func_envcreator = get_env_creator_script(config.type);
  if (is_null(func_envcreator)) {
    last_del_eid_ = eid;
    return SCRIPT_EID_INVALID;
  }
  Interface *env = func_envcreator();
  env->set_workpath(config.workpath);
  env->set_rootpath(config.rootpath);
  std::unique_ptr< Interface > pointer(env);
  envs_[eid] = std::move(pointer);
  return eid;
}

eid_t Factory::newenv(Interface* env) {
  eid_t eid = neweid();
  if (SCRIPT_EID_INVALID == eid) return eid;
  std::unique_ptr< Interface > pointer(env);
  envs_[eid] = std::move(pointer);
  return eid;
}

eid_t Factory::neweid() {
  std::unique_lock<std::mutex> autolock(mutex_);
  eid_t eid = SCRIPT_EID_INVALID;
  eid = SCRIPT_EID_INVALID == last_del_eid_ ?
        static_cast<eid_t>(envs_.size() + 1) : last_del_eid_;
  last_del_eid_ = SCRIPT_EID_INVALID;
  return eid;
}
