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

set_compiler_flags_for_external_libraries()
restore_compiler_flags()

set(gtest_incdir ${dependencies_gtest_dir}/include)
if(EXISTS "${dependencies_gtest_dir}/../../third_party")
  set(gtest_hack_incdir "${dependencies_gtest_dir}/../..")
endif()
set(gtest_libdir ${dependencies_gtest_dir})

if (MSVC)
  option(PF_BUILD_AS_DLL "Build plain framwork as dll." ON)
endif()

# Include helper functions and macros used by Google Test.
include(${gtest_libdir}/cmake/internal_utils.cmake)
config_compiler_and_linker()
# string(REPLACE "-W4" "-W3" cxx_default "${cxx_default}")
string(REPLACE "-Wshadow" "" cxx_default "${cxx_default}")
string(REPLACE "-Wextra" "" cxx_default "${cxx_default}")

# This is the directory into which the executables are built.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${gtest_incdir}
                    ${gtest_hack_incdir}
                    ${plainframework_dir}/include/
                    ${root_dir}/plain/app/include/
                    ${CMAKE_CURRENT_LIST_DIR})

# Common libraries for tests.
if(NOT MSVC)
  find_package(Threads)
endif()
set(COMMON_LIBS "pf_core;${CMAKE_THREAD_LIBS_INIT}")

 # Plain Framework core flags.
set(cxx_base_flags "${cxx_base_flags} -DPF_CORE -DPF_OPEN_EPOLL")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

#For utf8 no boom.
if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4819")
endif()

# Generate a rule to build a unit test executable ${test_name} with
# source file ${source}.  For details of additional arguments, see
# mathfu_configure_flags().
function(test_executable test_name source)
  cxx_executable_with_flags(${test_name} "${cxx_base_flags} ${cxx_default}" "${COMMON_LIBS}"
    ${source} ${PLAINFRAMEWORK_HEADERS})
  plainframework_configure_flags(${test_name} ${ARGN})
  plainframework_enable_warnings(${test_name})
endfunction()

# Generate a rule to build unit test executables.
function(test_executables test_name source)
  # Default build options for the target architecture.
  test_executable(${test_name} "${source}")
  MESSAGE(${source})
endfunction()

file(GLOB_RECURSE CORE_TEST_SOURCES "../src/*.cc")

test_executables(../../bin/app "${CORE_TEST_SOURCES}")
