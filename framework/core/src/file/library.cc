#include <algorithm>
#include "pf/basic/logger.h"
#include "pf/file/library.h"

typedef void (__stdcall *function_open)(void *);

/* The diffrent os library prefix and suffix. */
#if OS_WIN
#define LIBRARY_PREFIX ""
#define LIBRARY_SUFFIX ".dll"
#include "pf/basic/ts_string.h"
/* Routines to convert from UTF8 to native Windows text */
#if UNICODE
#define WIN_StringToUTF8(S) ts_strdup_unicode_to_ascii(S)
#define WIN_UTF8ToString(S) (WCHAR *)ts_strdup_ascii_to_unicode(S)
#else
#error Not Implemented
#endif
static inline std::string GetLastErrorString(DWORD nErrorCode) {
  WCHAR *msg = 0; //Qt: wchar_t
  // Ask Windows to prepare a standard message for a GetLastError() code:
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_FROM_SYSTEM | 
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                nErrorCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&msg,
                0,
                NULL);
  char *amsg = ts_strdup_unicode_to_ascii(msg);
  std::string s(amsg);
  free(amsg);
  LocalFree(msg);
  return s;
}
#elif OS_MAC
#define LIBRARY_PREFIX "lib"
#define LIBRARY_SUFFIX ".dylib"
#include <dlfcn.h>
#elif OS_UNIX
#define LIBRARY_PREFIX "lib"
#define LIBRARY_SUFFIX ".so"
#include <dlfcn.h>
#endif

std::unique_ptr< pf_file::LibraryManager > g_librarymanager{nullptr};

namespace pf_file {

void Library::set_filename(const std::string &_filename) {
  filename_ = _filename;
  if (_filename.size() < strlen(LIBRARY_SUFFIX)) {
    filename_ += LIBRARY_SUFFIX;
    return;
  }
  size_t suf_pos = _filename.rfind(LIBRARY_SUFFIX); //find the last: xxx.soyz.so.1
  if (suf_pos == std::string::npos) {
    filename_ += LIBRARY_SUFFIX;
    return;
  }
  // xx.sox, xx.so.1
  if (suf_pos + strlen(LIBRARY_SUFFIX) > _filename.size()) {
    if (_filename.substr(suf_pos + strlen(LIBRARY_SUFFIX), 1) != ".") {
      filename_ += LIBRARY_SUFFIX;
      return;
    }
  }
}

bool Library::load(bool tryprefix) {
#if OS_WIN
  tryprefix = false;
  LPTSTR tstr = ts_strdup_ascii_to_unicode(filename_.c_str());
  handle_ = cast(void *, LoadLibrary(tstr));
  free(tstr);
  if (is_null(handle_)) 
    errorstr_ = GetLastErrorString(GetLastError());
#else
  handle_ = dlopen(filename_.c_str(), RTLD_NOW | RTLD_LOCAL);
  if (is_null(handle_)) errorstr_ = dlerror();
#endif
  if (is_null(handle_)) {
    if (!tryprefix || 
        0 == strlen(LIBRARY_PREFIX) || 
        filename_.substr(0, strlen(LIBRARY_PREFIX)) == LIBRARY_PREFIX) {
#if _DEBUG
      SLOW_ERRORLOG("library", 
                    "[library] load (%s) error: %s", 
                    filename_.c_str(), 
                    errorstr_.c_str());
#endif
      return false;
    }
    filename_ = LIBRARY_PREFIX + filename_;
    return load(false);
  } else {
    errorstr_.clear();
  }
  isloaded_ = !is_null(handle_);
#if _DEBUG
  SLOW_DEBUGLOG("library", 
                "[library] load(%s) handle: %p", 
                filename_.c_str(), 
                handle_);
#endif
  return !is_null(handle_);
}
   
bool Library::unload() {
#if OS_WIN
  if (!is_null(handle_)) {
    FreeLibrary((HMODULE)handle_);
  }
  DWORD err = GetLastError();
  if (0 == err) {
    errorstr_.clear();
    handle_ = nullptr;
  } else {
    errorstr_ = GetLastErrorString(err);
  }
#else
  if (!is_null(handle_)) {
    int32_t err = dlclose(handle_);
    if (err != 0) {
      errorstr_ = dlerror();
    } else {
      errorstr_.clear();
      handle_ = nullptr;
    }
  }
#endif
  return is_null(handle_);
}
   
void *Library::resolve(const std::string &symbol, bool again) {
  void *symbolhandle = nullptr;
#if OS_WIN
  symbolhandle = cast(void *, GetProcAddress((HMODULE)handle, symbol.c_str()));
  if (is_null(symbolhandle)) {
    errorstr_ = GetLastErrorString(GetLastError());
    SLOW_ERRORLOG("library", 
                  "[library] resolve(%s) failed, error: %s", 
                  symbol.c_str(), 
                  errorstr_.c_str());
  }
#else
  symbolhandle = dlsym(handle_, symbol.c_str());
  if (is_null(symbolhandle))
    errorstr_ = dlerror();
#endif
  if (is_null(symbolhandle) && again) {
    return resolve("_" + symbol, false);
  }
  if (symbolhandle) {
    errorstr_.clear();
  }
  return symbolhandle;
}

LibraryManager::LibraryManager() {
  add_searchpaths({"./"});
#if OS_UNIX
  add_searchpaths({
      "/lib/", 
      "/lib64/",
      "/user/lib/", 
      "/user/lib64/", 
      "/user/local/lib/",
      "/user/lobal/lib64"
  });
#endif
}

template<> LibraryManager *
  pf_basic::Singleton< LibraryManager >::singleton_ = nullptr;

LibraryManager *LibraryManager::getsingleton_pointer() {
  return singleton_;
}

LibraryManager &LibraryManager::getsingleton() {
  Assert(singleton_);
  return *singleton_;
}

void LibraryManager::add_searchpaths(const std::vector<std::string> &paths) {
  for (const std::string &path : paths) {
    if (std::find(searchpaths_.begin(), 
                  searchpaths_.end(), 
                  path) == searchpaths_.end())
      searchpaths_.push_back(path);
  }
}

void LibraryManager::remove_searchpaths(const std::vector<std::string> &paths) {
  for (const std::string &path : paths) {
    searchpaths_.erase(
        std::remove(searchpaths_.begin(), searchpaths_.end(), path), 
        searchpaths_.end());
  }
}

void LibraryManager::remove_librarynames(const std::string &onlyname, 
                                         const std::vector<std::string> &names) {
  std::vector<std::string> list = namesmap_[onlyname];
  for (const std::string &name : names) {
    list.erase(std::remove(list.begin(), list.end(), name), list.end());
  }
}
   
bool LibraryManager::load(const std::string &name, 
                          const pf_basic::type::variable_array_t &params) {
  if (librarymap_[name]) {
    SLOW_DEBUGLOG("library", "[library] load(%s) has loaded", name.c_str());
    return true;
  }
  std::vector<std::string> names = namesmap_[name];
  if (0 == names.size()) names.push_back(name);
  auto library = new Library();
  for (const std::string &_name : names) {
    library->set_filename(_name);
    if (library->load()) break;
    SLOW_ERRORLOG("library", 
                  "[library] load(%s) error-> %s", 
                  _name.c_str(), 
                  library->errorstr().c_str());
  }
  if (!library->isloaded()) {
    safe_delete(library);
    return false;
  }

  //Main enter function.
  std::string mainfunc{name};
  if (mainfunc.find(".") != std::string::npos)
    mainfunc.replace(mainfunc.find("."), mainfunc.size() - 1, "");
  mainfunc = "pfopen_" + mainfunc;
  auto openhanlde = library->resolve(mainfunc);
  if (!is_null(openhanlde)) {
    auto openfunc = (function_open)openhanlde;
    openfunc(cast(void *, &params));
  }

  librarymap_[name] = nullptr;
  std::unique_ptr<Library> &pointer = librarymap_[name];
  unique_move(pf_file::Library, library, pointer);
  SLOW_DEBUGLOG("library", "[library] load(%s) ok!", name.c_str());
  return true;
}

bool LibraryManager::unload(const std::string &name) {
  auto it = librarymap_.find(name);
  if (it == librarymap_.end()) {
    SLOW_DEBUGLOG("library", "[library] load(%s) has unloaded", name.c_str());
    return true;
  }
  if (!it->second->unload()) {
    SLOW_ERRORLOG("library", 
                  "[library] load(%s) error: %s", 
                  name.c_str(), 
                  it->second->errorstr().c_str());
    return false;
  }
  librarymap_.erase(name);
  return true;
}

} //namespace pf_file
