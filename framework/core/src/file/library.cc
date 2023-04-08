#include "plain/file/library.h"
#include <filesystem>
#include "plain/basic/logger.h"
#include "plain/basic/utility.h"
#include "plain/basic/global.h"
#include "plain/file/api.h"
#include "plain/engine/kernel.h"

PLAIN_SINGLETON_DECL(plain::LibraryManager);

using namespace plain;

typedef void (__stdcall *function_open)(plain::Kernel*, void*);

/* The diffrent os library prefix and suffix. */
#if OS_WIN
#define LIBRARY_PREFIX ""
#define LIBRARY_SUFFIX ".dll"

static inline std::string GetLastErrorString(DWORD nErrorCode) {
  using namespace plain_basic::string;
  WCHAR* msg = 0; //Qt: wchar_t
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
  std::string s = wstr2str(msg);
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

void Library::set_filename(const std::string& _filename) {
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

bool Library::load(bool tryprefix, bool seeglb) {
  std::filesystem::path path{path_ + filename_};
  auto fileexists = std::filesystem::exists(path);
  if (fileexists) {
#if OS_WIN
    UNUSED(seeglb);
    tryprefix = false;
#ifdef _UNICODE
    auto wstr = str2wstr(temp);
    handle_ = cast(void*, LoadLibrary(wstr.c_str()));
#else
    handle_ = cast(void*, LoadLibrary(temp.c_str()));
#endif
    if (is_null(handle_)) 
      errorstr_ = GetLastErrorString(GetLastError());
#else
    handle_ = dlopen(
        path.c_str(), RTLD_NOW | (seeglb ? RTLD_GLOBAL : RTLD_LOCAL));
    if (is_null(handle_)) errorstr_ = dlerror();
#endif
  }
  if (is_null(handle_)) {
    if (!tryprefix || 
        0 == strlen(LIBRARY_PREFIX) || 
        filename_.substr(0, strlen(LIBRARY_PREFIX)) == LIBRARY_PREFIX) {
#if _DEBUG
      if (fileexists) {
        /*
        SLOW_ERRORLOG("library", 
                    "[library] load (%s) error: %s", 
                    filename_.c_str(), 
                    errorstr_.c_str());
        */
      }
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
   
void* Library::resolve(const std::string& symbol, bool again) {
  void* symbolhandle = nullptr;
#if OS_WIN
  symbolhandle = cast(void *, GetProcAddress((HMODULE)handle_, symbol.c_str()));
  if (is_null(symbolhandle)) {
    errorstr_ = GetLastErrorString(GetLastError());
    /*
    SLOW_ERRORLOG("library", 
                  "[library] resolve(%s) failed, error: %s", 
                  symbol.c_str(), 
                  errorstr_.c_str());
    */
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
  add_searchpaths({
      "./",
      GLOBALS["app.basepath"].data,
      GLOBALS["app.basepath"].data + "plugins/",
  });
#if OS_UNIX
  add_searchpaths({
      "/lib/", 
      "/lib64/",
      "/usr/lib/", 
      "/usr/lib64/", 
      "/usr/local/lib/",
      "/usr/local/lib64/"
  });
#endif
}

LibraryManager::~LibraryManager() = default;

void LibraryManager::add_searchpaths(const std::vector<std::string> &paths) {
  for (const std::string& path : paths) {
    if (std::find(searchpaths_.begin(), 
                  searchpaths_.end(), 
                  path) == searchpaths_.end())
      searchpaths_.push_back(path);
  }
}

void LibraryManager::remove_searchpaths(const std::vector<std::string> &paths) {
  for (const std::string& path : paths) {
    searchpaths_.erase(
        std::remove(searchpaths_.begin(), searchpaths_.end(), path), 
        searchpaths_.end());
  }
}

void LibraryManager::remove_librarynames(const std::string& onlyname, 
                                         const std::vector<std::string> &names) {
  std::vector<std::string> list = namesmap_[onlyname];
  for (const std::string& name : names) {
    list.erase(std::remove(list.begin(), list.end(), name), list.end());
  }
}
   
bool LibraryManager::load(const std::string& name, 
                          bool seeglb,
                          const variable_array_t &params) {
  if (librarymap_[name]) {
    // SLOW_DEBUGLOG("library", "[library] load(%s) has loaded", name.c_str());
    return true;
  }
  std::vector<std::string> names = namesmap_[name];
  if (0 == names.size()) names.push_back(name);
  std::unique_ptr<Library> library(new Library());
  for (const std::string& _name : names) {
    bool loaded{false};
    library->set_filename(_name);
    for (const std::string& path : searchpaths_) {
      library->set_path(path);
      if (library->load(true, seeglb)) {
        loaded = true;
        break;
      }
    }
#if OS_WIN
    // Try load debug files.
    if (!loaded) {
      std::string debug_name = _name + "d";
      library->set_filename(debug_name);
      for (const std::string& path : searchpaths_) {

        library->set_path(path);
        if (library->load(true, seeglb)) {
          loaded = true;
          break;
        }
      }
    }
#endif
    if (!loaded) {
      std::string err{"The file mybe not found!"};
      if (library->errorstr() != "")
        err = library->errorstr();
      /*
      SLOW_ERRORLOG("library", 
                    "[library] load(%s) error-> %s", 
                    _name.c_str(),
                    err.c_str());
      */
    }
  }
  if (!library->isloaded()) {
    return false;
  }

  //Main enter function.
  std::string mainfunc{name};
  if (mainfunc.find(".") != std::string::npos)
    mainfunc.replace(mainfunc.find("."), mainfunc.size() - 1, "");
  mainfunc = "plainopen_" + mainfunc;
  auto openhanlde = library->resolve(mainfunc);
  if (!is_null(openhanlde)) {
#ifdef __GNUC__
__extension__
#endif
    function_open openfunc = reinterpret_cast<function_open>(openhanlde);
    // openfunc(ENGINE_POINTER, cast(void *, &params));
  }

  librarymap_[name] = nullptr;
  librarymap_[name] = std::move(library);
  // SLOW_DEBUGLOG("library", "[library] load(%s) ok!", name.c_str());
  return true;
}

bool LibraryManager::unload(const std::string& name) {
  auto it = librarymap_.find(name);
  if (it == librarymap_.end()) {
    // SLOW_DEBUGLOG("library", "[library] load(%s) has unloaded", name.c_str());
    return true;
  }
  if (!it->second->unload()) {
    /*
    SLOW_ERRORLOG("library", 
                  "[library] load(%s) error: %s", 
                  name.c_str(), 
                  it->second->errorstr().c_str());
    */
    return false;
  }
  librarymap_.erase(name);
  return true;
}
