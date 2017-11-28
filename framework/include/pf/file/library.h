/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id library.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/06/30 11:35
 * @uses Some useful with library.
 *       refer code: https://github.com/wang-bin/dllapi.
*/
#ifndef PF_FILE_LIBRARY_H_
#define PF_FILE_LIBRARY_H_

#include "pf/file/config.h"
#include "pf/basic/singleton.tcc"
#include "pf/basic/type/variable.h"

namespace pf_file {

class PF_API Library {

 public:
   Library(const std::string &filename = "") : 
     filename_{filename},
     isloaded_{false},
     handle_{nullptr} {};
   ~Library() { unload(); };

 public:
   std::string errorstr() const { return errorstr_; };
   std::string filename() const { return filename_; }
   void set_filename(const std::string &filename);

 public:
   bool isloaded() const { return isloaded_; };
   bool load(bool tryprefix = true);
   bool unload();
   void *resolve(const std::string &symbol, bool again = true);

 private:
   std::string errorstr_;
   std::string filename_;
   bool isloaded_;
   void *handle_;

};

class PF_API LibraryManager : public pf_basic::Singleton< LibraryManager > {

 public:
   LibraryManager();
   ~LibraryManager() {};

 public:
   static LibraryManager &getsingleton();
   static LibraryManager *getsingleton_pointer();

 public:
   
   /* Set the search paths can find the librarys. */
   void set_searchpaths(const std::vector<std::string> &paths) {
     searchpaths_ = paths;
   }

   /* Add the search paths can find the librarys. */
   void add_searchpaths(const std::vector<std::string> &paths);

   /* Remove the search paths can find the librarys. */
   void remove_searchpaths(const std::vector<std::string> &paths);

   /* Get all current search paths. */
   std::vector<std::string> get_searchpaths() {
     return searchpaths_;
   }

   /* Set the load library all possible names. 
    * e.g onlyname "OPGL"
    *     names "OpenGL", "OpenGL32"
    * */
   void set_librarynames(const std::string &onlyname, 
                         const std::vector<std::string> &names) {
     namesmap_[onlyname] = names;
   }

   /* Remove the load library all possible names(look up set). */
   void remove_librarynames(const std::string &onlyname, 
                            const std::vector<std::string> &names);

   /* Get the load library all possible names. */
   std::vector<std::string> get_librarynames(const std::string &onlyname) {
     return namesmap_[onlyname];
   }

   /* Load the library by name. */
   bool load(const std::string &name, 
             const pf_basic::type::variable_array_t &params = {});

   /* Unload the library by name. */
   bool unload(const std::string &name);

   /* Get the library pointer by name. */
   Library *get(const std::string &name) {
     return is_null(librarymap_[name]) ? nullptr : librarymap_[name].get();
   }

 private:
   
   /* The search paths. */
   std::vector<std::string> searchpaths_;

   /* The names map. */
   std::map<std::string, std::vector<std::string> > namesmap_;

   /* The library object pointer map. */
   std::map<std::string, std::unique_ptr<Library> > librarymap_;

};

}; //namespace pf_file

extern std::unique_ptr< pf_file::LibraryManager > g_librarymanager;
#define LIBRARY_MANAGER_POINTER pf_file::LibraryManager::getsingleton_pointer()

#endif //PF_FILE_LIBRARY_H_
