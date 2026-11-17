cmake_minimum_required(VERSION 3.22)

project(Priorisea
  VERSION 1.0
  DESCRIPTION "SEANERGYS priority manager"
  LANGUAGES C CXX)

# ---- Options
option(PRIORISEA_PCH    "Use precompiled headers to speed up compilation" ON)
option(ENABLE_COVERAGE  "Enable code coverage instrumentation with gcov"  OFF)
option(BUILD_TESTING    "Build the test dependencies"                     ON)

# ---- Global settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "build type" FORCE)
endif()

include(GNUInstallDirs)
include(FindPkgConfig)
enable_testing()

if(ENABLE_COVERAGE)
  add_compile_options(--coverage -O0 -g)
  add_link_options(--coverage)
endif()

find_package(Threads REQUIRED)

# ---- Dependencies
pkg_check_modules(JANSSON   REQUIRED IMPORTED_TARGET jansson)
pkg_check_modules(SEANERBUS REQUIRED IMPORTED_TARGET seanerbus-c)
pkg_check_modules(FLUX_CORE REQUIRED IMPORTED_TARGET flux-core>=0.81.0)
pkg_check_modules(SQLITE3   REQUIRED IMPORTED_TARGET sqlite3)

# CLI11
find_path(CLI11_INCLUDE_DIR CLI/CLI.hpp REQUIRED)
add_library(CLI11::CLI11 INTERFACE IMPORTED)
set_target_properties(CLI11::CLI11 PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${CLI11_INCLUDE_DIR}")

# tinyexpr
add_library(tinyexpr STATIC external/tinyexpr/tinyexpr.c)
target_include_directories(tinyexpr PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/external/tinyexpr)

# External headers
set(PRIORISEA_EXT_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/external/tinyexpr
  ${CMAKE_CURRENT_SOURCE_DIR}/external/croncpp/include)

if(BUILD_TESTING)
  # demobus
  if(NOT DEFINED ENV{DEMOBUS_BIN})
    message(WARNING
      "demobus not found. Set DEMOBUS_BIN "
      "to run the SeanerBUS integration tests "
      "(e.g. tests/t3003-seanerbus.t); skipping for now.")
  endif()

  # naming server
  if(NOT DEFINED ENV{NAMING_SERVER_BIN})
    message(WARNING
      "naming server binary not found. Set NAMING_SERVER_BIN to run the "
      "naming-server integration tests (e.g. tests/t3003-seanerbus.t); "
      "skipping for now.")
  endif()
endif()

# ---- Helper: attach the shared precompiled headers
function(priorisea_add_pch target)
  if(NOT PRIORISEA_PCH)
    return()
  endif()
  target_precompile_headers(${target} PRIVATE
    <string> <vector> <map> <memory>
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/error.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/types.hpp)
endfunction()

# ---- Build (lib)
add_library(priorisea_lib STATIC
  src/lib/error.cpp
  src/cli/endpoint/error.cpp
  src/daemon/endpoint/error.cpp
  src/database/error.cpp
  src/lib/formula.cpp
  src/lib/info.cpp
  src/lib/job.cpp
  src/lib/timewin.cpp
  src/lib/user.cpp
  src/lib/util.cpp)

target_include_directories(priorisea_lib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PRIORISEA_EXT_INCLUDES})

target_link_libraries(priorisea_lib PUBLIC
  PkgConfig::JANSSON
  PkgConfig::FLUX_CORE
  tinyexpr)

priorisea_add_pch(priorisea_lib)

# ---- Build (CLI)
add_executable(priorisea
  # CLI
  src/cli/main.cpp
  src/cli/util.cpp
  src/cli/commands/daemon.cpp
  src/cli/commands/database.cpp
  src/cli/commands/formula.cpp
  src/cli/commands/job.cpp
  src/cli/commands/plugin.cpp
  src/cli/commands/timewin.cpp
  src/cli/commands/user.cpp
  src/cli/endpoint/util.cpp
  src/cli/endpoint/flux/formula.cpp
  src/cli/endpoint/flux/info.cpp
  src/cli/endpoint/flux/job.cpp
  src/cli/endpoint/flux/plugin.cpp
  src/cli/endpoint/flux/timewin.cpp
  src/cli/endpoint/flux/user.cpp
  # Daemon
  src/daemon/daemon.cpp
  src/daemon/endpoint/flux/endpoint.cpp
  src/daemon/endpoint/flux/callback/formula.cpp
  src/daemon/endpoint/flux/callback/info.cpp
  src/daemon/endpoint/flux/callback/job.cpp
  src/daemon/endpoint/flux/callback/plugin.cpp
  src/daemon/endpoint/flux/callback/timewin.cpp
  src/daemon/endpoint/flux/callback/user.cpp
  # Database
  src/database/database.cpp
  src/database/formula.cpp
  src/database/info.cpp
  src/database/sqlite/formula.cpp
  src/database/sqlite/info.cpp
  src/database/sqlite/job.cpp
  src/database/sqlite/user.cpp
  src/database/sqlite/sqlite.cpp
  src/database/sqlite/timewin.cpp
  src/database/sqlite/user.cpp)

target_include_directories(priorisea PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PRIORISEA_EXT_INCLUDES})

target_link_libraries(priorisea PRIVATE
  priorisea_lib
  CLI11::CLI11
  PkgConfig::JANSSON
  PkgConfig::SEANERBUS
  PkgConfig::FLUX_CORE
  PkgConfig::SQLITE3
  tinyexpr
  Threads::Threads)

priorisea_add_pch(priorisea)

install(TARGETS priorisea RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Bash completion
set(PRIORISEA_COMPLETION_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/cli/priorisea.bash)
set(PRIORISEA_COMPLETION_DIR ${CMAKE_BINARY_DIR}/src/cli)
set(PRIORISEA_COMPLETION_OUT ${PRIORISEA_COMPLETION_DIR}/priorisea.bash)
add_custom_command(
  OUTPUT ${PRIORISEA_COMPLETION_OUT}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${PRIORISEA_COMPLETION_DIR}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${PRIORISEA_COMPLETION_SRC} ${PRIORISEA_COMPLETION_OUT}
  DEPENDS ${PRIORISEA_COMPLETION_SRC}
  COMMENT "Copying bash completion script")
add_custom_target(priorisea_completion ALL DEPENDS ${PRIORISEA_COMPLETION_OUT})
install(FILES ${PRIORISEA_COMPLETION_SRC}
  DESTINATION ${CMAKE_INSTALL_DATADIR}/bash-completion/completions
  RENAME priorisea)


# ---- Build (sea_priority)
add_library(sea_priority MODULE
  src/plugin/flux/sea_priority.cpp)

target_include_directories(sea_priority PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PRIORISEA_EXT_INCLUDES})

target_link_libraries(sea_priority PRIVATE
  priorisea_lib
  PkgConfig::SQLITE3)

priorisea_add_pch(sea_priority)

set_target_properties(sea_priority PROPERTIES
  PREFIX ""
  LIBRARY_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/lib/flux/job-manager/plugins)

install(TARGETS sea_priority LIBRARY DESTINATION lib/flux/job-manager/plugins)

# ---- Tests
if(BUILD_TESTING)
  # catch2
  find_path(CATCH2_INCLUDE_DIR catch2/catch.hpp REQUIRED)
  add_library(Catch2::Catch2 INTERFACE IMPORTED)
  set_target_properties(Catch2::Catch2 PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CATCH2_INCLUDE_DIR}")

  add_library(catchmain STATIC src/lib/test/main.cpp)
  target_include_directories(catchmain PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} ${PRIORISEA_EXT_INCLUDES} ${CATCH2_INCLUDE_DIR})
  target_link_libraries(catchmain PUBLIC priorisea_lib Catch2::Catch2)

  foreach(topic formula info job timewin user util)
    add_executable(${topic}_test src/lib/test/${topic}.cpp)
    target_link_libraries(${topic}_test PRIVATE
      catchmain
      PkgConfig::JANSSON
      PkgConfig::FLUX_CORE
      Catch2::Catch2)
    add_test(NAME ${topic} COMMAND ${topic}_test)
  endforeach()

  #sharness
  add_executable(sb_response
    tests/scripts/sb_response.cpp)

  target_link_libraries(sb_response PRIVATE
    PkgConfig::SEANERBUS
    Threads::Threads)

  set_target_properties(sb_response PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests/scripts)

  file(GLOB SHARNESS_TESTS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/t[0-9]*.t)

  foreach(t ${SHARNESS_TESTS})
    get_filename_component(tname ${t} NAME)
    add_test(NAME ${tname}
      COMMAND sh ${tname}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    set_tests_properties(${tname} PROPERTIES ENVIRONMENT
      "SHARNESS_BUILD_DIRECTORY=${CMAKE_BINARY_DIR};SHARNESS_TEST_SRCDIR=${CMAKE_CURRENT_SOURCE_DIR}/tests;PATH=${CMAKE_CURRENT_BINARY_DIR}/src/cli:$ENV{PATH}")
  endforeach()
endif()

# ---- Docs
find_program(DOXYGEN_EXECUTABLE doxygen)
find_program(SPHINX_BUILD_EXECUTABLE sphinx-build)

if(DOXYGEN_EXECUTABLE AND SPHINX_BUILD_EXECUTABLE)
  add_custom_target(docs
    COMMAND ${CMAKE_COMMAND} -E make_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/docs/doxygen/xml
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile
    COMMAND ${SPHINX_BUILD_EXECUTABLE} -b html
            ${CMAKE_CURRENT_SOURCE_DIR}/docs
            ${CMAKE_CURRENT_SOURCE_DIR}/docs/build
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs
    COMMENT "Building HTML documentation with Doxygen and Sphinx")
endif()
