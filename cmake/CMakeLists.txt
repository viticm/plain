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
cmake_minimum_required(VERSION 3.27.0)

set(PROJECT_NAME PlainFramework)
set(PLAIN_VERSION 2.0)
set(PROJECT_DESC "Plain framework, based on c++ for net applictions")

if (CMAKE_VERSION VERSION_LESS 3.0)
  project(PlainFramework CXX C)
else()
  cmake_policy(SET CMP0048 NEW)
  cmake_policy(SET CMP0037 NEW)
  project(PlainFramework VERSION ${PLAIN_VERSION} LANGUAGES CXX C)
endif()

# Call fplutil to get locations of dependencies and set common build settings.
include("inc/find_fplutil.cmake")
include("inc/common.cmake")
include("inc/internal_utils.cmake")

if (NOT dependencies_gtest_dir)
  set(dependencies_gtest_dir ${root_dir}/dependencies/googletest/googletest)
endif()

if (NOT has_output_path)

  # This is the directory into which the executables are built.
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${root_dir}/plain/bin)

  # This is the directory into which the librarys are built.
  set(LIBRARY_OUTPUT_PATH ${root_dir}/plain/lib)

  set(has_output_path 1)

endif()

# C++ Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#For utf8 no boom and standard.
if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++latest -wd4819")
endif()

# Options that control the build configuration.
# To configure PlainFramework flags per build target, see the
# plainframework_configure_flags() function.
option(plainframework_build_tests "Build PlainFramework unit tests." ON)

# Build plain framework plugins.
option(plainframework_build_plugins "Build PlainFramework plugins" ON)

file(GLOB_RECURSE PLAINFRAMEWORK_HEADERS ${CMAKE_CURRENT_LIST_DIR}/framework/core/include *.h)

set(VERSION_RC ${root_dir}/cmake/inc/version.rc.in)

add_subdir(${plainframework_dir}/cmake plainframework plainframework)

if(plainframework_build_tests)
  add_subdir(${root_dir}/framework/unit_tests/cmake
             ${root_dir}/framework/unit_tests/cmake/build
             plainframework)
  if (NOT plainframework_no_app)
    add_subdir(${root_dir}/plain/app/cmake
               ${root_dir}/plain/app/cmake/build
               plainframework)
  endif()
endif()

# Plugins.
# If put build plugin before tests then will have a read access permission
# conflicts on windows.
if (plainframework_build_plugins)
  #add_subdir(${root_dir}/plain/plugins/cmake plugins plainframework)
endif()
