#include "pf/cache/storeinterface.h"
#include "pf/cache/repository.h"

namespace pf_cache {

Repository::Repository(StoreInterface *_store) {
  store_ = _store;
  minutes_ = 0;
}

Repository::~Repository() {
  safe_delete(store_);
}

   
bool Repository::has(const char *key) {
  return !is_null(get(key));
}
   
void *Repository::get(const char *key, void *_default) {
  void *result = _default;
  if (is_null(store_)) return result;
  result = store_->get(key);
  if (is_null(result)) result = _default;
  return result;
}

void *Repository::pull(const char *key, void *_default) {
  if (is_null(store_)) return nullptr; 
  void *result = get(key, _default);
  store_->forget(key);
  return result;
}

void Repository::put(const char *key, void *value, int32_t minutes) {
  if (!is_null(store_)) {
    store_->put(key, value, minutes);
  }
}

void Repository::add(const char *key, void *value, int32_t minutes) {
  if (is_null(get(key))) put(key, value, minutes);
}

void Repository::forget(const char *key) {
  if (is_null(store_)) store_->forget(key);
}

void *Repository::remember(const char *key, 
                           int32_t minutes, 
                           function_callback callback) {
  void *result = nullptr;
  if (!is_null(result = get(key))) return result;
  if (!is_null(callback)) {
    put(key, result = (*callback)(), minutes);
  }
  return result;
}

void *Repository::sear(const char *key, function_callback callback) {
  return remember_forever(key, callback);
}

void *Repository::remember_forever(const char *key, 
                                   function_callback callback) {
  if (is_null(store_)) return nullptr; 
  void *result{nullptr};
  if (!is_null(result = get(key))) return result;
  if (!is_null(callback)) { 
    store_->forever(key, result = (*callback)());
  }
  return result;
}

int32_t Repository::get_default_cachetime() const {
  return minutes_;
}

void Repository::set_default_cachetime(int32_t minutes) {
  minutes_ = minutes;
}

void Repository::set_cache_time(const char *key, int32_t minutes) {
  if (!is_null(store_)) store_->set_cache_time(key, minutes);
}
   
void Repository::tick() {
  if (!is_null(store_)) store_->tick();
}

bool Repository::set_cache_count(int32_t count) {
  bool result = false;
  if (!is_null(store_)) result = store_->set_cache_count(count);
  return result;
}

void Repository::recycle(const char *key) {
  if (!is_null(store_)) store_->recycle(key);
}

} //namespace pf_cache
