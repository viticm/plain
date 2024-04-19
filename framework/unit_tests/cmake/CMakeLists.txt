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
cmake_minimum_required(VERSION 2.8)

if (MSVC)
  # For utf8 no boom.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4819")
endif()

# For gtest new(future will change it to googletest dir).
set(GOOGLETEST_VERSION 1.11.0)

# Plain framework crt.
if (plain_build_shared AND MSVC)
  option(BUILD_SHARED_LIBS "plainframework build shared" ON)
endif()

add_subdir(${dependencies_gtest_dir} gtest googletest)

set(gtest_incdir ${dependencies_gtest_dir}/include)
if(EXISTS "${dependencies_gtest_dir}/../../third_party")
  set(gtest_hack_incdir "${dependencies_gtest_dir}/../..")
endif()
set(gtest_libdir ${dependencies_gtest_dir})

config_compiler_and_linker()

# string(REPLACE "-W4" "-W3" cxx_default "${cxx_default}")
# string(REPLACE "-Wshadow" "" cxx_default "${cxx_default}")
# string(REPLACE "-Wextra" "" cxx_default "${cxx_default}")

include_directories(${gtest_incdir}
                    ${gtest_hack_incdir}
                    ${plainframework_dir}/include/
                    ${root_dir}/framework/unit_tests/core_test/
                    ${CMAKE_CURRENT_LIST_DIR})

# Common libraries for tests.
if(NOT MSVC)
  find_package(Threads)
endif()

if (MSVC)
  set(COMMON_LIBS "plain;gtest;${CMAKE_THREAD_LIBS_INIT}")
else()
  set(COMMON_LIBS "plain;gtest;${CMAKE_THREAD_LIBS_INIT}")
endif()

# Plain Framework flags.
set(cxx_base_flags "${cxx_base_flags} -DPLAIN_OPEN_EPOLL")
if (MSVC AND plain_build_shared)
  set(cxx_base_flags "${cxx_base_flags} -DPLAIN_BUILD_AS_DLL")
endif()

set(CMAKE_CXX_STANDARD 23)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
endif()

# Generate a rule to build a unit test executable ${test_name} with
# source file ${source}.  For details of additional arguments, see
# mathfu_configure_flags().
function(test_executable test_name source)
  cxx_executable_with_flags(${test_name} "${cxx_base_flags} ${cxx_default}" "${COMMON_LIBS}"
    ${source} ${PLAINFRAMEWORK_HEADERS})
  plainframework_configure_flags(${test_name} ${ARGN})
  plainframework_enable_warnings(${test_name})
  target_link_libraries(${test_name} ${COMMON_LIBS})
  if (BUILD_SHARED_LIBS)
    set_target_properties(${test_name}
      PROPERTIES
      COMPILE_DEFINITIONS "GTEST_LINKED_AS_SHARED_LIBRARY=1")
  endif()
endfunction()

# Generate a rule to build unit test executables.
function(test_executables test_name source)
  # Default build options for the target architecture.
  test_executable(${test_name}_tests "${source}")
  set_target_properties(${test_name}_tests PROPERTIES DEBUG_POSTFIX "d")
  # MESSAGE(${source})
endfunction()

# Enable this then just test the directories.
#set(TEST_DIRS "net")

if (NOT TEST_DIRS)
  file(GLOB_RECURSE CORE_TEST_SOURCES "../core_test/*.cc")
else()
  file(GLOB CORE_TEST_SOURCES "../core_test/*.cc")
  foreach(dir ${TEST_DIRS})
    file(GLOB_RECURSE CORE_TEST_OTHER_SOURCES "../core_test/${dir}/*.cc")
    set(CORE_TEST_SOURCES ${CORE_TEST_SOURCES} ${CORE_TEST_OTHER_SOURCES})
  endforeach()
endif()

test_executables(core "${CORE_TEST_SOURCES}")

# Copy gtest libraries.
if (MSVC AND plain_build_shared AND BUILD_SHARED_LIBS)
  if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_custom_command(TARGET core_tests
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/bin/gtestd.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
  else()
    add_custom_command(TARGET core_tests
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/bin/gtest.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
  endif()
endif()
