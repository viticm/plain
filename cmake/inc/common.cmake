# Defines functions and macros useful for building Plain Framework.
#
# Note:
#
# - This file will be run twice when building Plain core and tests (once via
#   Plain Framework's CMakeLists.txt).
#   Therefore it shouldn't have any side effects other than defining
#   the functions and macros.

#For install paths.
if (CMAKE_VERSION VERSION_LESS 2.8.5)
  set(CMAKE_INSTALL_BINDIR "bin" CACHE STRING "User executables (bin)")
  set(CMAKE_INSTALL_LIBDIR "lib${LIB_SUFFIX}" CACHE STRING "Object code libraries (lib)")
  set(CMAKE_INSTALL_INCLUDEDIR "include" CACHE STRING "C header files (include)")
  mark_as_advanced(CMAKE_INSTALL_BINDIR CMAKE_INSTALL_LIBDIR CMAKE_INSTALL_INCLUDEDIR)
else()
  include(GNUInstallDirs)
endif()
# Save the plainframework directory, store this in the cache so that it's globally
# accessible from plainframework_configure_flags().
set(root_dir ${CMAKE_CURRENT_LIST_DIR}/../.. CACHE INTERNAL "plainframework root directory")
set(plainframework_dir ${root_dir}/framework/core CACHE INTERNAL "plainframework directory")
set(dependencies_dir ${root_dir}/dependencies CACHE INTERNAL "plainframework dependencies directory")

# Modify CMAKE_C_FLAGS and CMAKE_CXX_FLAGS to enable a maximum reasonable
# warning level.
function(plainframework_enable_warnings target)
  get_target_property(target_compile_flags ${target} COMPILE_FLAGS)
  if(MSVC)
    # C4127: conditional expression is constant
    # C4577: 'noexcept' used with no exception handling mode specified.
    target_compile_options(${target} PRIVATE /W4 /WX /wd4127 /wd4577)
  elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR
      CMAKE_COMPILER_IS_CLANGXX)
    # Set the maximum warning level for gcc.
    target_compile_options(${target} PRIVATE -Wall -Wextra -Werror
      -Wno-long-long -Wno-variadic-macros)
  endif()
endfunction()

# Safe add_subdirectory.
function(add_subdir target target_build)
  set(old_root_dir ${root_dir} CACHE INTERNAL "cache")
  add_subdirectory(${target} ${target_build})
  set(root_dir ${old_root_dir} CACHE INTERNAL "recover")
endfunction(add_subdir)

# Sets and caches `var` to the first path in 'paths' that exists.
# If none of the paths are found, sets `var` to an error value of
# "path_not_found".
function(set_to_first_path_that_exists var paths description)
  foreach(path ${paths})
    if(EXISTS "${path}")
      set(${var} "${path}" CACHE PATH description)
      return()
    endif()
  endforeach(path)
  set(${var} "path_not_found_to_${var}" CACHE PATH description)
endfunction(set_to_first_path_that_exists)
