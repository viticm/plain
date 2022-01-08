#include "pf/basic/util.h"
#include "pf/basic/time_manager.h"
#include "pf/sys/thread.h"
#include "pf/basic/logger.h"

std::unique_ptr< pf_basic::Logger > g_logger{nullptr};

namespace pf_basic {

static std::mutex g_log_mutex;

template<> Logger *Singleton< Logger >::singleton_ = nullptr;

Logger *Logger::getsingleton_pointer() {
  return singleton_;
}

Logger &Logger::getsingleton() {
  Assert(singleton_);
  return *singleton_;
}

Logger::Logger() {
  logids_.init(LOGTYPE_MAX);
  log_position_.init(LOGTYPE_MAX);
  logcache_.init(LOGTYPE_MAX);
  loglock_.init(LOGTYPE_MAX);
  cache_size_ = 0;
}

Logger::~Logger() {
  cache_size_ = 0;
  for (auto it = logcache_.begin(); it != logcache_.end(); ++it)
    safe_delete_array(it->second);
  for (auto it = loglock_.begin(); it != loglock_.end(); ++it)
    safe_delete(it->second);
}

std::mutex& Logger::get_mutex() {
  return g_log_mutex;
}
   
bool Logger::register_fastlog(const char *logname) {
  uint8_t count = static_cast<uint8_t>(logids_.getcount());
  if (count > logids_.get_maxcount()) return false;
  if (logids_.isfind(logname)) return false;
  uint8_t logid = count + 1;
  logids_.add(logname, logid);
  log_position_.add(logid, 0);
  auto cache = new char[kDefaultLogCacheSize];
  memset(cache, 0, kDefaultLogCacheSize);
  logcache_.add(logid, cache);
  auto mutex = new std::mutex;
  loglock_.add(logid, mutex);
  if (is_null(logcache_.get(logid))) return false;
  return true;
}

void Logger::get_log_timestr(char *time_str, int32_t length) {
  if (TIME_MANAGER_POINTER) {
      TIME_MANAGER_POINTER->reset_time();
      auto runtime = TIME_MANAGER_POINTER->get_run_time();
      snprintf(
          time_str, 
          length, 
          "%.2d:%.2d:%.2d (%s %.3f)",
          TIME_MANAGER_POINTER->get_hour(),
          TIME_MANAGER_POINTER->get_minute(),
          TIME_MANAGER_POINTER->get_second(),
          pf_sys::thread::get_id().c_str(),
          static_cast< double >(runtime) / 1000);
  } else {
    snprintf(time_str,
             length, 
             "00:00:00 (%s 0.000)",
             pf_sys::thread::get_id().c_str());
  }
}

bool Logger::init(int32_t cache_size) {
  if (GLOBALS["log.clear"] == 1) {
    char command[128] = {0};
#if OS_UNIX
    snprintf(command, 
             sizeof(command) - 1, 
             "rm -rf %s/*.log", 
             GLOBALS["log.directory"].c_str());
#elif OS_WIN
    snprintf(command, 
             sizeof(command) - 1, 
             "del %s/*.log", 
             GLOBALS["log.directory"].c_str());
    util::path_towindows(command, static_cast<uint16_t>(strlen(command)));
#endif
    system(command);
    return true;
  }
  cache_size_ = cache_size;
  return true;
}

void Logger::get_log_filename(const char *filename_prefix, 
                           char *save, 
                           uint8_t type) { 
  const char *typestr{nullptr};
  switch (type) {
    case 1:
      typestr = "warning";
      break;
    case 2:
      typestr = "error";
      break;
    case 3:
      typestr = "debug";
      break;
    default:
      typestr = "";
      break;
  }
  char prefixfinal[128] = {0};
  snprintf(prefixfinal, 
           sizeof(prefixfinal) - 1, 
           "%s%s%s",
           filename_prefix,
           strlen(typestr) > 0 ? "_" : "",
           typestr);
  if (TIME_MANAGER_POINTER) {
    char savedir[128] = {0};
    snprintf(savedir, 
             sizeof(savedir) - 1, 
             "%s/%.2d_%.2d_%.2d/%s", 
             GLOBALS["log.directory"].c_str(),
             TIME_MANAGER_POINTER->get_year(), 
             TIME_MANAGER_POINTER->get_month(),
             TIME_MANAGER_POINTER->get_day(),
             GLOBALS["app.name"].c_str());
    if (!pf_basic::util::makedir(savedir, 0755))
      io_cerr("save dir: %s make failed", savedir);
    snprintf(save,
             FILENAME_MAX - 1,
             "%s/%s_%.2d.log",
             savedir,
             prefixfinal,
             TIME_MANAGER_POINTER->get_hour());
  } else {
    snprintf(save,
             FILENAME_MAX - 1,
             "%s/%s%s%s.log",
             GLOBALS["log.directory"].c_str(),
             filename_prefix,
             strlen(typestr) > 0 ? "_" : "",
             typestr);
  }
}

void Logger::flush_log(const char *logname) {
    uint8_t logid = static_cast<uint8_t>(logids_.get(logname));
    char *buffer = logcache_.get(logid);
    uint32_t position = log_position_.get(logid);
    if (!loglock_.isfind(logid) || is_null(buffer)) return;
    char log_filename[FILENAME_MAX];
    memset(log_filename, '\0', sizeof(log_filename));
    get_log_filename(logname, log_filename);
    auto mutex = loglock_.get(logid);
    std::unique_lock<std::mutex> autolock(*mutex);
    try {
      FILE* fp;
      fp = fopen(log_filename, "ab");
      if (fp) {
        fwrite(buffer, 1, position, fp);
        fclose(fp);
      }
    } catch(...) {
      //do nothing
    }
}

void Logger::flush_alllog() {
    logids_t::iterator_t iterator;
    for (iterator = logids_.begin(); iterator != logids_.end(); ++iterator) {
      flush_log(iterator->first.c_str());
    }
}

void Logger::remove_log(const char *file_name) {
  std::unique_lock<std::mutex> autolock(g_log_mutex);
  FILE* fp;
  fp = fopen(file_name, "w");
  if (fp) fclose(fp);
}

} //namespace pf_basic
