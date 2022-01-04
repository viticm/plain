# Copyright 2017 Viticm. All rights reserved.
#
# Licensed under the MIT License(the "License");
# you may not use this file except in compliance with the License.
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
cmake_minimum_required(VERSION 2.8.12)

set(PROJECT_NAME PlainFramework)

if (CMAKE_VERSION VERSION_LESS 3.0)
  project(PlainFramework CXX C)
else()
  cmake_policy(SET CMP0048 NEW)
  cmake_policy(SET CMP0037 NEW)
  project(PlainFramework VERSION 1.9.0 LANGUAGES CXX C)
endif()

# Call fplutil to get locations of dependencies and set common build settings.
include("inc/find_fplutil.cmake")
include("inc/common.cmake")
include("${fplutil_dir}/buildutil/cmake_common.txt")
set_common_build_variables()

# This is the directory into which the executables are built.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${root_dir}/plain/bin)

# This is the directory into which the librarys are built.
SET(LIBRARY_OUTPUT_PATH ${root_dir}/plain/lib)

#For utf8 no boom.
if (MSVC)                                                                          
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4819")                                
endif()

# Options that control the build configuration.
# To configure PlainFramework flags per build target, see the
# plainframework_configure_flags() function.
option(plainframework_build_tests "Build PlainFramework unit tests." ON)

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

file(GLOB_RECURSE PLAINFRAMEWORK_HEADERS ${CMAKE_CURRENT_LIST_DIR}/framework/core/include *.h)

#add_subdirectory(${root_dir}/framework/core/cmake ${root_dir}/framework/unit_tests/core/build)
add_subdirectory(${plainframework_dir}/cmake plainframework)

if(plainframework_build_tests)
  add_subdirectory(${root_dir}/framework/unit_tests/cmake
                   ${root_dir}/framework/unit_tests/cmake/build)
  add_subdirectory(${root_dir}/plain/app/cmake
                   ${root_dir}/plain/app/cmake/build)

endif()
