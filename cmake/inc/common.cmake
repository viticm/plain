# Defines functions and macros useful for building Plain Framework.
#
# Note:
#
# - This file will be run twice when building Plain core and tests (once via
#   Plain Framework's CMakeLists.txt).
#   Therefore it shouldn't have any side effects other than defining
#   the functions and macros.

# For install paths.
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

function(plainframework_configure_flags target)

  # Add required includes to the target.
  target_include_directories(${target}
    PRIVATE ${plainframework_dir}/include)

  # Parse optional arguments.
  set(additional_args ${ARGN})
  list(LENGTH additional_args num_additional_args)
  if(${num_additional_args} GREATER 0)
    list(GET additional_args 0 enable_simd)
  endif()
  if(${num_additional_args} GREATER 1)
    list(GET additional_args 1 force_padding)
  endif()
endfunction()

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

# Macro defined here so that it can be used by all projects included
macro(plainframework_set_ios_attributes project)
  if(fpl_ios)
    set_target_properties(${project} PROPERTIES
      XCODE_ATTRIBUTE_SDKROOT "iphoneos")
    set_target_properties(${project} PROPERTIES
      XCODE_ATTRIBUTE_ARCHS "$(ARCHS_STANDARD)")
    set_target_properties(${project} PROPERTIES
      XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "NO")
    set_target_properties(${project} PROPERTIES
      XCODE_ATTRIBUTE_VALID_ARCHS "$(ARCHS_STANDARD)")
    set_target_properties(${project} PROPERTIES
      XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "8.0")
  endif()
endmacro(plainframework_set_ios_attributes)

# External code should be compiled with these compiler options.
# Call this function before add_subdirectory([external project]), and then
# call restore_comiler_flags() after.
function(set_compiler_flags_for_external_libraries)
  # Save current compiler flags so that we can restore them in
  # restore_compiler_flags()
  set(SAVED_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" PARENT_SCOPE)
  set(SAVED_CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}" PARENT_SCOPE)
  set(SAVED_CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}" PARENT_SCOPE)
  set(SAVED_CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE}" PARENT_SCOPE)
  set(SAVED_CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" PARENT_SCOPE)
  set(SAVED_CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}" PARENT_SCOPE)
  set(SAVED_CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}" PARENT_SCOPE)

  # Detect clang
  if(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
    set(using_clangxx 1)
  endif()

  # Suppress all warnings.
  if( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR
      using_clangxx OR APPLE )
    set(CMAKE_CXX_FLAGS "-w -Wno-deprecated-declarations" PARENT_SCOPE)
    set(CMAKE_C_FLAGS "-w" PARENT_SCOPE)
  elseif(MSVC)
    set(CMAKE_C_FLAGS "${MSVC_FLAGS}" PARENT_SCOPE)
    set(CMAKE_C_FLAGS_DEBUG "${MSVC_FLAGS_DEBUG}" PARENT_SCOPE)
    set(CMAKE_C_FLAGS_RELEASE "${MSVC_FLAGS_RELEASE}" PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS "${MSVC_FLAGS}" PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS_DEBUG "${MSVC_FLAGS_DEBUG}" PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS_RELEASE "${MSVC_FLAGS_RELEASE}" PARENT_SCOPE)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${MSVC_LINKER_FLAGS_RELEASE}" PARENT_SCOPE)
    add_definitions("${MSVC_DEFINES}")
  endif()
endfunction(set_compiler_flags_for_external_libraries)

# Restore the compiler flags to the values they had before
# set_compiler_flags_for_external_libraries() was called.
function(restore_compiler_flags)
  set(CMAKE_CXX_FLAGS "${SAVED_CMAKE_CXX_FLAGS}" PARENT_SCOPE)
  set(CMAKE_CXX_FLAGS_DEBUG "${SAVED_CMAKE_CXX_FLAGS_DEBUG}" PARENT_SCOPE)
  set(CMAKE_CXX_FLAGS_RELEASE "${SAVED_CMAKE_CXX_FLAGS_RELEASE}" PARENT_SCOPE)
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${SAVED_CMAKE_EXE_LINKER_FLAGS_RELEASE}" PARENT_SCOPE)
  set(CMAKE_C_FLAGS "${SAVED_CMAKE_C_FLAGS}" PARENT_SCOPE)
  set(CMAKE_C_FLAGS_DEBUG "${SAVED_CMAKE_C_FLAGS_DEBUG}" PARENT_SCOPE)
  set(CMAKE_C_FLAGS_RELEASE "${SAVED_CMAKE_C_FLAGS_RELEASE}" PARENT_SCOPE)
endfunction()

# Safe add_subdirectory.
function(add_subdir target target_build project)
  set_compiler_flags_for_external_libraries()
  set(saved_root_dir${project} ${root_dir} CACHE INTERNAL "root dir cache")
  add_subdirectory(${target} ${target_build})
  set(root_dir ${saved_root_dir${project}} CACHE INTERNAL "root dir recover")
  restore_compiler_flags()
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
