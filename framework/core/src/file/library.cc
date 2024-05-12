#include "plain/file/library.h"
#include <filesystem>
#include "plain/basic/logger.h"
#include "plain/basic/utility.h"
#include "plain/basic/global.h"
#include "plain/file/api.h"
#include "plain/engine/kernel.h"

using namespace plain;

typedef void (__stdcall *function_open)(plain::Kernel *, void *);

/* The diffrent os library prefix and suffix. */
#if OS_WIN
#define LIBRARY_PREFIX ""
#define LIBRARY_SUFFIX ".dll"

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
  std::string s = wstr2str(msg);
  LocalFree(msg);
  return s;
}

#elif OS_MAC
#define LIBRARY_PREFIX "lib"
#define LIBRARY_SUFFIX ".dylib"
#include <dlfcn.h>
#elif OS_UNIX || OS_MAC
#define LIBRARY_PREFIX "lib"
#define LIBRARY_SUFFIX ".so"
#include <dlfcn.h>
#endif

void Library::set_filename(const std::string &_filename) noexcept {
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

bool Library::load(bool tryprefix, bool seeglb) noexcept {
  std::filesystem::path path{path_ + filename_};
  auto fileexists = std::filesystem::exists(path);
  if (fileexists) {
#if OS_WIN
    UNUSED(seeglb);
    tryprefix = false;
#ifdef _UNICODE
    auto wstr = str2wstr(path.string());
    handle_ = cast(void *, LoadLibrary(wstr.c_str()));
#else
    handle_ = cast(void *, LoadLibrary(temp.c_str()));
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
        LOG_ERROR << "[library] load (" << filename_ << ") error: "
          << errorstr_;
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
  LOG_DEBUG << "load: " << filename_;
#endif
  return !is_null(handle_);
}
   
bool Library::unload() noexcept {
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
   
void *Library::resolve(const std::string &symbol, bool again) noexcept {
  void *symbolhandle = nullptr;
#if OS_WIN
  symbolhandle = cast(void *, GetProcAddress((HMODULE)handle_, symbol.c_str()));
  if (is_null(symbolhandle)) {
    errorstr_ = GetLastErrorString(GetLastError());
    LOG_ERROR << "[library] resolve(" << symbol << ") failed, error: "
      << errorstr_;
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
      GLOBALS["app.basepath"].get<std::string>(),
      GLOBALS["app.basepath"].get<std::string>() + "plugins/",
  });
#if OS_UNIX || OS_MAC
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

void LibraryManager::add_searchpaths(
  const std::vector<std::string> &paths) noexcept {
  for (const std::string &path : paths) {
    if (std::find(searchpaths_.begin(), 
                  searchpaths_.end(), 
                  path) == searchpaths_.end())
      searchpaths_.push_back(path);
  }
}

void LibraryManager::remove_searchpaths(
  const std::vector<std::string> &paths) noexcept {
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
   
bool LibraryManager::load(
  const std::string &name, bool seeglb,
  const variable_array_t &params) noexcept {
  if (librarymap_[name]) {
    LOG_DEBUG << "[library] load(" << name << ")  has loaded";
    return true;
  }
  std::vector<std::string> names = namesmap_[name];
  if (0 == names.size()) names.push_back(name);
  std::unique_ptr<Library> library(new Library());
  for (const std::string &_name : names) {
    bool loaded{false};
    library->set_filename(_name);
    for (const std::string &path : searchpaths_) {
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
      for (const std::string &path : searchpaths_) {

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
      LOG_ERROR << "[library] load(" << name << ") error: " << err;
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
    openfunc(ENGINE.get(), cast(void *, &params));
  }

  librarymap_[name] = nullptr;
  librarymap_[name] = std::move(library);
  LOG_DEBUG << "[library] load(" << name << ") ok!";
  return true;
}

bool LibraryManager::unload(const std::string &name) noexcept {
  auto it = librarymap_.find(name);
  if (it == librarymap_.end()) {
    LOG_DEBUG << "[library] unload(" << name << ") not loaded";
    return true;
  }
  if (!it->second->unload()) {
    LOG_ERROR << "[library] load(" << name << ") error: "
      << it->second->errorstr();
    return false;
  }
  librarymap_.erase(name);
  return true;
}
