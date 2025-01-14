/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 * $Id library.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2023/04/01 22:38
 * @uses Some useful with library.
 *       refer code: https://github.com/wang-bin/dllapi.
*/
#ifndef PLAIN_FILE_LIBRARY_H_
#define PLAIN_FILE_LIBRARY_H_

#include "plain/file/config.h"
#include <unordered_map>
#include "plain/basic/singleton.h"
#include "plain/basic/type/variable.h"

namespace plain {

class PLAIN_API Library {

 public:
   Library(const std::string &_filename = "") : 
     filename_{_filename},
     isloaded_{false},
     handle_{nullptr} {};
   ~Library() { unload(); };

 public:
   std::string errorstr() const noexcept { return errorstr_; };
   std::string filename() const noexcept { return filename_; }
   void set_filename(const std::string &filename) noexcept;
   void set_path(const std::string &path) noexcept {
     path_ = path;
   };

 public:
   bool isloaded() const noexcept { return isloaded_; };
   bool load(bool tryprefix = true, bool seeglb = false) noexcept;
   bool unload() noexcept;
   void *resolve(const std::string &symbol, bool again = true) noexcept;

 private:
   std::string errorstr_;
   std::string filename_;
   std::string path_;
   bool isloaded_{false};
   void *handle_{nullptr};

};

class PLAIN_API LibraryManager {

 public:
  LibraryManager();
  ~LibraryManager();

 public:
   
  /* Set the search paths can find the librarys. */
  void set_searchpaths(const std::vector<std::string> &paths) noexcept {
    searchpaths_ = paths;
  }

  /* Add the search paths can find the librarys. */
  void add_searchpaths(const std::vector<std::string> &paths) noexcept;

  /* Remove the search paths can find the librarys. */
  void remove_searchpaths(const std::vector<std::string> &paths) noexcept;

  /* Get all current search paths. */
  std::vector<std::string> get_searchpaths() const noexcept {
    return searchpaths_;
  }

  /* Set the load library all possible names. 
  * e.g onlyname "OPGL"
  *     names "OpenGL", "OpenGL32"
  * */
  void set_librarynames(
    const std::string &onlyname, const std::vector<std::string> &names) {
    namesmap_[onlyname] = names;
  }

  /* Remove the load library all possible names(look up set). */
  void remove_librarynames(
    const std::string &onlyname, const std::vector<std::string> &names);

  /* Get the load library all possible names. */
  std::vector<std::string>
  get_librarynames(const std::string &onlyname) noexcept {
    return namesmap_[onlyname];
  }

  /* Load the library by name. */
  bool load(
    const std::string &name, bool seeglb = false,
    const variable_array_t &params = {}) noexcept;

  /* Unload the library by name. */
  bool unload(const std::string &name) noexcept;

  /* Get the library pointer by name. */
  Library *get(const std::string &name) noexcept {
    return is_null(librarymap_[name]) ? nullptr : librarymap_[name].get();
  }

 private:
  
  /* The search paths. */
  std::vector<std::string> searchpaths_;

  /* The names map. */
  std::unordered_map<std::string, std::vector<std::string>> namesmap_;

  /* The library object pointer map. */
  std::unordered_map<std::string, std::shared_ptr<Library>> librarymap_;

};

} //namespace plain
  
#ifndef LIBRARY
#define LIBRARY plain::Singleton<plain::LibraryManager>::get_instance()
#endif

#endif //PLAIN_FILE_LIBRARY_H_
