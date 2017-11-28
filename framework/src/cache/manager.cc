#include "pf/cache/db_store.h"
#include "pf/cache/repository.h"
#include "pf/cache/manager.h"

namespace pf_cache {

Manager::Manager() : db_dirver_{nullptr} {

}
   
Manager::~Manager() {

}

Repository *Manager::create_db_dirver() {
  Repository *result = 
    is_null(db_dirver_) ? new Repository(new DBStore) : db_dirver_.get();
  if (is_null(db_dirver_)) {
    std::unique_ptr< Repository > pointer(result);
    db_dirver_ = std::move(pointer);
  }
  return result;
}
   
Repository *Manager::get_db_dirver() {
  return db_dirver_.get();
}

} //namespace pf_cache
