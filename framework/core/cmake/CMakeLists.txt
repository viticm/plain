########################################################################
# CMake build script for Plain Framework.
#
# To run the tests for Plain Framework itself on Linux, use 'make test' or
# ctest.  You can select which tests to run using 'ctest -R regex'.
# For more options, run 'ctest --help'.

########################################################################
#
# Project-wide settings
cmake_minimum_required(VERSION 2.8.12)

# Name of the project.
#
# CMake files in this project can refer to the root source directory
# as ${PLAIN_SOURCE_DIR} and to the root binary directory as
# ${PLAIN_BINARY_DIR}.
# Language "C" is required for find_package(Threads).
if (CMAKE_VERSION VERSION_LESS 3.0)
  project(plain CXX C)
else()
  cmake_policy(SET CMP0048 NEW)
  project(plain VERSION ${PLAIN_VERSION} LANGUAGES CXX C)
endif()

# When other libraries are using a shared version of runtime libraries,
# Plain Framework also has to use one.
option(
  plain_force_shared_crt
  "Use shared (DLL) run-time lib even when Plain Framework is built as static lib."
  OFF)

option(plain_build_tests "Build all of plain's own tests." OFF)

option(plain_build_shared "Build all of plain as share library." ON)

option(plain_build_samples "Build plain's sample programs." OFF)

option(plain_disable_pthreads "Disable uses of pthreads in plain." OFF)

option(plain_with_uring "Build plain with liburing(just on unix)." OFF)

option(
  plain_hide_internal_symbols
  "Build plain with internal symbols hidden in shared libraries."
  OFF)

set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Generate debug library name with a postfix.")

# Windows crt.
set(plain_force_shared_crt ${plain_build_shared})

if (COMMAND config_compiler_and_linker)
  config_compiler_and_linker()
endif()

# Defines pre_project_set_up_hermetic_build() and set_up_hermetic_build().
include(inc/hermetic_build.cmake OPTIONAL)

if (COMMAND pre_project_set_up_hermetic_build)
  pre_project_set_up_hermetic_build()
endif()

if (POLICY CMP0063) # Visibility
  cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

if (COMMAND set_up_hermetic_build)
  set_up_hermetic_build()
endif()

if (plain_hide_internal_symbols)
  set(CMAKE_CXX_VISIBILITY_PRESET hidden)
  set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
endif()

set(PLAIN_SOURCE_DIR "..")

# Where Plain Framework's .h files can be found.
include_directories(
  "${PLAIN_SOURCE_DIR}/include"
  "${PLAIN_SOURCE_DIR}")

# Summary of tuple support for Microsoft Visual Studio:
# Compiler    version(MS)  version(cmake)  Support
# ----------  -----------  --------------  -----------------------------
# <= VS 2010  <= 10        <= 1600         Use Plain Frameworks's own tuple.
# VS 2012     11           1700            std::tr1::tuple + _VARIADIC_MAX=10
# VS 2013     12           1800            std::tr1::tuple
# VS 2015     14           1900            std::tuple
# VS 2017     15           >= 1910         std::tuple
if (MSVC AND MSVC_VERSION EQUAL 1700)
  add_definitions(/D _VARIADIC_MAX=10)
endif()

########################################################################
#
# Defines the plain libraries.  User tests should link
# with one of them.

set(INSTALL_PLAIN ON)
file(GLOB_RECURSE LIB_SOURCES "${PLAIN_SOURCE_DIR}/src/*.cc")
file(GLOB_RECURSE LIB_SOURCES_C "${PLAIN_SOURCE_DIR}/src/*.c")

if (MSVC)
  set(WITH_LIBS "ws2_32")
else()
  if (plain_with_uring)
    set(WITH_LIBS "dl;uring.a") # Lib uring use static link.
    set(core_flags "${core_flags} -DPLAIN_LIBURING_ENABLE")
  else()
    set(WITH_LIBS "dl")
  endif()
endif()

# This is the directory into which the executables are built.
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${root_dir}/plain/bin)

# Version rc.
if (MSVC AND VERSION_RC)
  set(plain_version_rc "${CMAKE_BINARY_DIR}/plain.rc")
  configure_file(${VERSION_RC} ${plain_version_rc})
endif()

if (WIN32)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
endif()

# Plain Framework libraries.  We build them using more strict warnings than what
# are used for other targets, to ensure that plain can be compiled by a user
# aggressive about warnings.
if (plain_build_shared)
  cxx_shared_library(plain "${core_flags} ${cxx_default} -DPLAIN_CORE" ${LIB_SOURCES} ${LIB_SOURCES_C} ${plain_version_rc})
  target_link_libraries(plain PUBLIC ${WITH_LIBS})
else()
  cxx_library(plain "${core_flags} ${cxx_strict}" ${LIB_SOURCES} ${LIB_SOURCES_C})
endif()

set_target_properties(plain PROPERTIES DEFINE_SYMBOL PLAIN_BUILD_AS_DLL)
set_target_properties(plain PROPERTIES SOVERSION 1)

# If the CMake version supports it, attach header directory information
# to the targets for when we are part of a parent build (ie being pulled
# in via add_subdirectory() rather than being a standalone build).
if (DEFINED CMAKE_VERSION AND NOT "${CMAKE_VERSION}" VERSION_LESS "2.8.12")
  target_include_directories(plain SYSTEM INTERFACE "${PLAIN_SOURCE_DIR}/include")
endif()

set(CMAKE_CXX_STANDARD 23)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-expansion-to-defined")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif()

#For utf8 no boom.
if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4819")
endif()

#For install paths.
if (CMAKE_VERSION VERSION_LESS 2.8.5)
  set(CMAKE_INSTALL_BINDIR "bin" CACHE STRING "User executables (bin)")
  set(CMAKE_INSTALL_LIBDIR "lib${LIB_SUFFIX}" CACHE STRING "Object code libraries (lib)")
  set(CMAKE_INSTALL_INCLUDEDIR "include" CACHE STRING "C header files (include)")
  mark_as_advanced(CMAKE_INSTALL_BINDIR CMAKE_INSTALL_LIBDIR CMAKE_INSTALL_INCLUDEDIR)
else()
  include(GNUInstallDirs)
endif()

########################################################################
#
# Install rules
if(INSTALL_PLAIN)
  install(TARGETS plain
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
  install(DIRECTORY "${PLAIN_SOURCE_DIR}/include/plain"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

  # configure and install pkgconfig files
  configure_file(
    inc/plain.pc.in
    "${CMAKE_BINARY_DIR}/plain.pc"
    @ONLY)
  install(FILES "${CMAKE_BINARY_DIR}/plain.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
endif()

########################################################################
#
# Samples on how to link user tests with plain or plain_main.
#
# They are not built by default.  To build them, set the
# plain_build_samples option to ON.  You can do it by running ccmake
# or specifying the -Dplain_build_samples=ON flag when running cmake.

if (plain_build_samples)

endif()

########################################################################
#
# Plain Framework's own tests.
#
# You can skip this section if you aren't interested in testing
# Plain Framework itself.
#
# The tests are not built by default.  To build them, set the
# plain_build_tests option to ON.  You can do it by running ccmake
# or specifying the -Dplain_build_tests=ON flag when running cmake.

if (plain_build_tests)
  # This must be set in the root directory for the tests to be run by
  # 'make test' or ctest.
  enable_testing()

  ############################################################
  # C++ tests built with standard compiler flags.

  #cxx_test(plain-death-test_test plain_main)

  ############################################################
  # C++ tests built with non-standard compiler flags.

  # MSVC 7.1 does not support STL with exceptions disabled.
  if (NOT MSVC OR MSVC_VERSION GREATER 1310)

  endif()


  if (NOT MSVC OR MSVC_VERSION LESS 1600)  # 1600 is Visual Studio 2010.
    # Visual Studio 2010, 2012, and 2013 define symbols in std::tr1 that
    # conflict with our own definitions. Therefore using our own tuple does not
    # work on those compilers.

  endif()

  ############################################################
  # Python tests.

  # Visual Studio .NET 2003 does not support STL with exceptions disabled.
  if (NOT MSVC OR MSVC_VERSION GREATER 1310)  # 1310 is Visual Studio .NET 2003

  endif()

  # MSVC 7.1 does not support STL with exceptions disabled.
  if (NOT MSVC OR MSVC_VERSION GREATER 1310)

  endif()

endif()
