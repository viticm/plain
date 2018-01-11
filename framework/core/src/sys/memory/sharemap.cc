#include "pf/sys/memory/sharemap.h"

using namespace pf_sys::memory::share;
_map_node_struct::_map_node_struct() :
  hash{0},
  prev{INDEX_INVALID},
  next{INDEX_INVALID} {
}
  
void _map_node_struct::clear() {
  prev = INDEX_INVALID;
  hash = 0;
  next = INDEX_INVALID;
}  

map_bucket_struct::map_bucket_struct() :
  cur(INDEX_INVALID) {
}
  
void map_bucket_struct::clear() {
  cur = INDEX_INVALID;
}

MapPool::MapPool() {
  //do nothing
}

MapPool::~MapPool() {
  //do nothing
}

bool MapPool::init(uint32_t _key, 
                   size_t _size, 
                   size_t datasize, 
                   bool create) {
  if (ready_) return true;
  set_data_extend_size(datasize);
  ref_obj_pointer_ = std::unique_ptr<share::Base>(new Base());
  Assert(ref_obj_pointer_);
  if (!ref_obj_pointer_) return false;
  bool result = true;
  bool needinit = false;
  auto headersize = sizeof(header_t);
  auto bucketsize = sizeof(map_bucket_t) * _size;
  auto full_datasize = (sizeof(map_node_t) + data_extend_size_) * _size;
  auto memorysize = headersize + bucketsize + full_datasize;
  result = ref_obj_pointer_->attach(_key, memorysize, false);
  if (create && !result) {
    result = ref_obj_pointer_->create(_key, memorysize);
    needinit = true;
  } else if (!result) {
    return false;
  }
  if (!result && GLOBALS["app.cmdmodel"] == kCmdModelClearAll) {
    return true;
  } else if (!result) {
    SLOW_ERRORLOG(
        "sharememory", 
        "[sys.memory.sharemap] (MapPool::init) failed");
    Assert(result);
    return result;
  }    
  size_ = _size;
  objs_ = new map_node_t * [size_];
  if (is_null(objs_)) return false;
  map_bucket_t *buckets = reinterpret_cast<map_bucket_t *>(getbuckets());
  for (decltype(size_) i = 0; i < size_; ++i) {
	auto nodesize = static_cast<uint32_t>(sizeof(map_node_t) + data_extend_size_);
    char *pointer = getdata(nodesize, static_cast<uint32_t>(i));
    objs_[i] = reinterpret_cast<map_node_t *>(pointer);
    if (is_null(objs_[i])) {
      Assert(false);
      return false;
    }
    objs_[i]->set_extend_size(data_extend_size_);
    if (data_extend_size_ > 0 && needinit) {
      memset(&pointer[sizeof(map_node_t)], 0, data_extend_size_);
    }
    if (needinit) {
      buckets[i].clear();
      objs_[i]->init();
    }
  }    
  key_ = _key;
  ready_ = true;
  return true;
}

map_node_t *MapPool::new_obj() {
  Assert(ref_obj_pointer_);
  auto header = ref_obj_pointer_->header();
  if (header->pool_position >= size_) return nullptr;
  auto obj = objs_[header->pool_position];
  obj->set_pool_id(static_cast<int32_t>(header->pool_position));
  ++(header->pool_position);
  obj->set_key(key_);
  return obj;
}

void MapPool::delete_obj(map_node_t *obj) {
  Assert(obj != nullptr && ref_obj_pointer_ != nullptr);
  header_t *header = ref_obj_pointer_->header();
  Assert(header); Assert(header->pool_position > 0);
  if (is_null(obj) || static_cast<int32_t>(header->pool_position) < 0) {
    return;
  }
  auto delete_index = obj->get_pool_id(); //this function 
                                          //must define in T*
  Assert(delete_index < static_cast<int32_t>(header->pool_position));
  if (delete_index >= static_cast<int32_t>(header->pool_position)) {
    return;
  }
  --(header->pool_position);
  map_bucket_t *buckets = reinterpret_cast<map_bucket_t *>(getbuckets());
  if (delete_index == static_cast<int32_t>(header->pool_position)) return;
  map_node_t *node = objs_[delete_index];

  //Safe to swap list.
  map_node_t *swapnode = objs_[header->pool_position];
  uint32_t datasize = static_cast<uint32_t>(sizeof(map_node_t) + data_extend_size_);
  char *pointer = reinterpret_cast<char *>(node);
  char *swappointer = reinterpret_cast<char *>(swapnode);
  memcpy(pointer, swappointer, datasize);
  node->set_pool_id(delete_index);

  //Safe to change the swap link list.
  if (node->data.prev != ID_INVALID) { //Prev node
    map_node_t *prevnode = get_obj(node->data.prev);
    unique_lock< map_node_t > auto_lock(*prevnode, kFlagMixedWrite);
    prevnode->data.next = delete_index;
  }
  if (node->data.next != INDEX_INVALID) { //Next node
    map_node_t *nextnode = get_obj(node->data.next);
    unique_lock< map_node_t > auto_lock(*nextnode, kFlagMixedWrite);
    nextnode->data.prev = delete_index;
  }

  //Safe to swap bucket.
  uint32_t _bucketindex = bucketindex(objs_[delete_index]->data.hash);
  if (buckets[_bucketindex].cur == 
      static_cast<int32_t>(header->pool_position)) {
    buckets[_bucketindex].cur = delete_index;
  }
  swapnode->clear();
}

uint32_t MapPool::bucketindex(uint32_t hash) {
  uint32_t index = hash & (size() - 1);
  return index;
}
   
uint32_t MapPool::hashkey(const char *str) {
  uint32_t hash = 5381;
  while (*str) {
    hash = ((hash << 5) + hash) ^ *str++;
  }
  return hash;
}

char *MapPool::getdata(uint32_t _size, uint32_t index) {
  char *result = nullptr;
  if (!ref_obj_pointer_) return result;
  char *data = ref_obj_pointer_->get();
  auto bucketsize = sizeof(map_bucket_t) * size_;
  char *realdata = data + bucketsize;
  auto data_fullsize = (sizeof(map_node_t) + data_extend_size_) * size_;
  Assert(_size * index <= data_fullsize - _size);
  result = (0 == _size || _size * index > data_fullsize - _size) ? 
            nullptr : 
            realdata + _size * index;
  return result;
}

char *MapPool::getbuckets() {
  char *data = ref_obj_pointer_->get();
  return data;
}

Map::Map() :
  keysize_{0},
  valuesize_{0},
  pool_{nullptr},
  buckets_{nullptr},
  ready_{false} {
}

Map::~Map() {
}

bool Map::init(uint32_t _key, 
               size_t _size, 
               size_t keysize, 
               size_t valuesize,
               bool create) {
  if (ready_) return true;
  auto pool = new MapPool;
  unique_move(MapPool, pool, pool_);
  Assert(pool_);
  auto datasize = (keysize + 1) + (valuesize + 1);
  bool result = pool_->init(_key, _size, datasize, create);
  if (!result) return result;
  buckets_ = reinterpret_cast<map_bucket_t *>(pool_->getbuckets());
  Assert(buckets_);
  keysize_ = keysize;
  valuesize_ = valuesize;
  ready_ = true;
  return true;
}

void Map::clear() {
  keysize_ = 0;
  valuesize_ = 0;
  buckets_ = nullptr;
}
   
const char *Map::get(const char *key) {
  const char *result = nullptr;
  auto index = getref(key);
  if (index != INDEX_INVALID) {
    map_node_t *node = pool_->get_obj(index);
    auto valuepos = sizeof(map_node_t) + keysize_ + 2;
    result = reinterpret_cast<char *>(node) + valuepos;
  }
  return result;
}
   
bool Map::set(const char *key, const char *value) {
  auto index = getref(key);
  auto valuesize = strlen(value);
  map_node_t *node = nullptr;
  auto valuepos = sizeof(map_node_t) + keysize_ + 2;
  valuesize = valuesize > valuesize_ ? valuesize_ : valuesize;
  if (index != INDEX_INVALID) {
    node = pool_->get_obj(index);
    Assert(node);
    if (is_null(node)) return false;
    unique_lock< map_node_t > auto_lock(*node, kFlagMixedWrite);
    char *pointer = reinterpret_cast<char *>(node) + valuepos;
    memset(pointer, 0, valuesize_ + 1);
    memcpy(pointer, value, valuesize);
  } else {
    auto header = pool_->get_header();
    Assert(header);
    unique_lock<header_t> auto_lock(*header, kFlagMixedWrite);
    node = newnode(key, value);
    if (is_null(node)) return false;
    addnode(node);
  }
  return true;
}

void Map::remove(const char *key) {
  auto header = pool_->get_header();
  auto index = getref(key);
  unique_lock<header_t> auto_lock(*header, kFlagMixedWrite);
  map_node_t *node = nullptr;
  map_bucket_t *buckets = reinterpret_cast<map_bucket_t *>(pool_->getbuckets());
  uint32_t hash = 0;
  if (index != INDEX_INVALID) {
    node = pool_->get_obj(index);
    hash = node->data.hash;
    //Swap hash list.
    if (node->data.prev != INDEX_INVALID) { //Prev node
      map_node_t *prevnode = pool_->get_obj(node->data.prev);
      unique_lock< map_node_t > auto_lockp(*prevnode, kFlagMixedWrite);
      prevnode->data.next = node->data.next;
    }
    if (node->data.next != INDEX_INVALID) { //Next node
      map_node_t *nextnode = pool_->get_obj(node->data.next);
      unique_lock< map_node_t > auto_lockn(*nextnode, kFlagMixedWrite);
      nextnode->data.prev = node->data.prev;
    }

    uint32_t _bucketindex = pool_->bucketindex(hash);
    if (static_cast<int32_t>(_bucketindex) >= 0 && _bucketindex < pool_->size()) {
      map_bucket_t *bucket = &buckets[_bucketindex];
      if (bucket->cur == static_cast<int32_t>(node->get_pool_id())) 
        bucket->cur = node->data.next;
    }
    pool_->delete_obj(node);
  }
}
   
map_node_t *Map::newnode(const char *key, const char *value) {
  map_node_t *node = nullptr;
  auto keypos = sizeof(map_node_t);
  auto valuepos = keypos + keysize_ + 2;
  node = pool_->new_obj();
  auto pool_id = node->get_pool_id();
  if (is_null(node)) return node;
  node->clear();
  char *pointer = reinterpret_cast<char *>(node);
  auto keysize = strlen(key);
  auto valuesize = strlen(value);
  uint32_t hash = pool_->hashkey(key);
  keysize = keysize > keysize_ ? keysize_ : keysize;
  valuesize = valuesize > valuesize_ ? valuesize_ : valuesize;
  node->data.clear();
  unique_lock< map_node_t > auto_lock(*node, kFlagMixedWrite);
  node->set_pool_id(pool_id);
  node->data.hash = hash;
  memset(pointer + keypos, 0, keysize_ + 1);
  memcpy(pointer + keypos, key, keysize);
  memset(pointer + valuepos, 0, valuesize + 1);
  memcpy(pointer + valuepos, value, valuesize);
  return node;
}
   
void Map::addnode(map_node_t *node) {
  unique_lock< map_node_t > auto_lock(*node, kFlagMixedWrite);
  int32_t n = pool_->bucketindex(node->data.hash);
  map_bucket_t *buckets = reinterpret_cast<map_bucket_t *>(pool_->getbuckets());
  node->data.next = buckets[n].cur;
  if (node->data.next != INDEX_INVALID) {
    map_node_t *_node = pool_->get_obj(node->data.next);
    _node->data.prev = node->get_pool_id();
  }
  buckets[n].cur = node->get_pool_id();
}
   
int32_t Map::getref(const char *key) {
  int32_t result = INDEX_INVALID;
  auto hash = pool_->hashkey(key);
  map_bucket_t *buckets = reinterpret_cast<map_bucket_t *>(pool_->getbuckets());
  if (pool_->get_position() > 0) {
    uint32_t _bucketindex = pool_->bucketindex(hash);
    Assert(static_cast<int32_t>(_bucketindex) >= 0 && _bucketindex < pool_->size());
    map_bucket_t *bucket = &buckets[_bucketindex];
    uint32_t keypos = sizeof(map_node_t);
    int32_t index = bucket->cur;
    while (index != INDEX_INVALID) {
      map_node_t *node = pool_->get_obj(index);
      Assert(node);
      char *pointer = reinterpret_cast<char *>(node);
      if (node->data.hash == hash && 0 == strcmp(pointer + keypos, key)) {
        result = index;
        break;
      }
      index = node->data.next;
    }
  }
  return result;
}

void map_iterator::generate_data() {
  if (is_null(map_) || !map_->is_ready()) return;
  if (current_ >= map_->getpool()->get_position()) {
    first = second = nullptr;
    return;
  }
  map_node_t *node = map_->getpool()->get_obj(current_);
  if (is_null(node)) {
    first = second = nullptr;
    return;
  }
  auto pointer = reinterpret_cast<char *>(node);
  first = pointer + sizeof(map_node_t);
  auto valuepos = sizeof(map_node_t) + map_->key_size() + 2;
  second = pointer + valuepos;
}

void map_reverse_iterator::generate_data() {
  if (is_null(map_) || !map_->is_ready()) return;
  if (current_ >= map_->getpool()->get_position()) {
    first = second = nullptr;
    return;
  }
  map_node_t *node = map_->getpool()->get_obj(current_);
  if (is_null(node)) {
    first = second = nullptr;
    return;
  }
  auto pointer = reinterpret_cast<char *>(node);
  first = pointer + sizeof(map_node_t);
  auto valuepos = sizeof(map_node_t) + map_->key_size() + 2;
  second = pointer + valuepos;
}
