#include "pf/sys/assert.h"
#include "pf/events/bus.h"

using namespace pf_events;
#undef max

size_t Bus::process() {
  return process_limit(std::numeric_limits<std::size_t>::max());
}

size_t Bus::process_limit(const size_t limit) {
  size_t process_count{0};
  std::lock_guard<std::mutex> write_guard_process{process_mutex_};
  std::vector< std::unique_ptr<pf_interfaces::events::Stream> > event_streams;
  {
    std::lock_guard<std::mutex> write_guard{streams_mutex_};
    std::swap(event_streams, event_streams_);
  }
  // Now if any setStream would be called it doesn't conflict without our 
  // process call.
  for (auto &event_stream : event_streams) {
    const auto run_process_count = event_stream->process(limit);
    process_count += run_process_count;
    if (process_count >= limit) break;
  }
  { 
    std::lock_guard<std::mutex> write_guard{streams_mutex_};
    if(!event_streams_.empty()) {
      // If anything was added then we need to add those elements
      std::move(event_streams_.begin(), 
          event_streams_.end(), std::back_inserter(event_streams));
    }
    std::swap(event_streams, event_streams_); // Move data to member

    // Check do we need remove something
    if(event_streams_.size() != event_to_stream_.size()) {
      auto remove_from = std::remove_if(
          event_streams_.begin(), 
          event_streams_.end(), [this](
            const std::unique_ptr<pf_interfaces::events::Stream> 
            &event_stream) {
          for (const auto &element : event_to_stream_) {
            // Don't remove if we point to the same place (is it UB ?)
            if (element.second == event_stream.get()) return false;
          }
          return true;
      });
      Assert(remove_from != event_streams_.end());
      event_streams_.erase(remove_from, event_streams_.end());
    }
  }
  return process_count;
}

pf_interfaces::events::Stream *Bus::find_stream(const id_t event_id) const {
  std::unique_lock<std::mutex> read_guard{streams_mutex_};
  return find_stream_unsafe(event_id);
}

void Bus::unlisten_all(const uint32_t listener_id) {
  std::unique_lock<std::mutex> read_guard{streams_mutex_};
  for (const auto &event_stream : event_to_stream_) {
    event_stream.second->remove_listener(listener_id);
  }
}

pf_interfaces::events::Stream *
Bus::find_stream_unsafe(const id_t event_id) const {
  auto lookup = event_to_stream_.find(event_id);
  return lookup != event_to_stream_.end() ? lookup->second : nullptr;
}

pf_interfaces::events::Stream *
Bus::obtain_stream(
    const id_t event_id, create_stream_callback_t create_stream_callback) {
  std::lock_guard<std::mutex> write_guard{streams_mutex_};
  auto *found = find_stream_unsafe(event_id);
  if (!is_null(found)) return found;
  auto stream = create_stream_callback();
  event_streams_.push_back(std::move(stream));
  event_to_stream_[event_id] = event_streams_.back().get();
  return event_streams_.back().get();
}

bool Bus::postpone_event(const PostponeHelper &call) {
  auto *event_stream = obtain_stream(call.id_, call.create_stream_callback_);
  event_stream->postpone(std::move(call.event_));
  return true;
}

pf_interfaces::events::Stream* 
Bus::listen(const uint32_t,
    const id_t event_id,
    create_stream_callback_t create_stream_callback) {
  auto *event_stream = obtain_stream(event_id, create_stream_callback);
  return event_stream;
}

void Bus::unlisten(const uint32_t listener_id, const id_t event_id) {
  pf_interfaces::events::Stream* event_stream = find_stream(event_id);
  if (!is_null(event_stream)) event_stream->remove_listener(listener_id);
}
