# SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
# SPDX-License-Identifier: MIT

#   mkdir -p build && cmake -S . -B build -G Ninja && cmake --build build
#   ctest --test-dir build            # run unit + integration tests
#
#   -D PRIORISEA_PCH=ON    precompiles the heavy headers (error.hpp/types.hpp
#                          chain + external headers) once instead of per TU. ON by default.

cmake_minimum_required(VERSION 3.22)

project(sea_priority_plugin
  VERSION 1.0
  DESCRIPTION "SEANERGYS flux priority plugin and priorisea CLI"
  LANGUAGES C CXX)

# ---- Options -------------------------------------------------------------
option(PRIORISEA_PCH    "Use precompiled headers to speed up compilation" ON)
option(ENABLE_COVERAGE  "enable code coverage instrumentation with gcov"   OFF)

# ---- Global settings -----------------------------------------------------
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

# ---- Dependencies (pkg-config) ------------------------------------------
# Kept even when the current CLI surface does not reference every library yet:
# the daemon/endpoint and database layers depend on them and they will be
# linked as that code is completed.
pkg_check_modules(JANSSON   REQUIRED IMPORTED_TARGET jansson)
pkg_check_modules(SEANERBUS REQUIRED IMPORTED_TARGET seanerbus-c)
pkg_check_modules(FLUX_CORE REQUIRED IMPORTED_TARGET flux-core>=0.81.0)
pkg_check_modules(SQLITE3   REQUIRED IMPORTED_TARGET sqlite3)

# CLI11
find_path(CLI11_INCLUDE_DIR CLI/CLI.hpp REQUIRED)
add_library(CLI11::CLI11 INTERFACE IMPORTED)
set_target_properties(CLI11::CLI11 PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${CLI11_INCLUDE_DIR}")

# Catch2 v2 (used by the unit tests, see commented test section below)
find_path(CATCH2_INCLUDE_DIR catch2/catch.hpp REQUIRED)
add_library(Catch2::Catch2 INTERFACE IMPORTED)
set_target_properties(Catch2::Catch2 PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${CATCH2_INCLUDE_DIR}")

# ---- External: tinyexpr -------------------------------------------------
add_library(tinyexpr STATIC external/tinyexpr/tinyexpr.c)
target_include_directories(tinyexpr PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/external/tinyexpr)

# ---- Threads -------------------------------------------------------------
find_package(Threads REQUIRED)

# ---- Helper: attach the shared precompiled headers ----------------------
function(priorisea_add_pch target)
  if(NOT PRIORISEA_PCH)
    return()
  endif()
  target_precompile_headers(${target} PRIVATE
    <string> <vector> <map> <memory>
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/error.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/types.hpp)
endfunction()

# Include roots covering every style used across the tree:
#   "src/..." / <src/...>  -> project root
#   <lib/...>              -> src/ root
#   "foo.hpp" (in src/lib) -> resolved relative to the TU itself
set(PRIORISEA_INCLUDE_ROOTS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Vendored headers (tinyexpr, croncpp) kept available for the formula/timewin
# layers even if they are not pulled in yet.
set(PRIORISEA_EXT_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/external/tinyexpr
  ${CMAKE_CURRENT_SOURCE_DIR}/external/croncpp/include)

# ---- priorisea CLI -------------------------------------------------------
# Single executable bundling the whole new src/ tree. The layers are tightly
# cross-referential (lib/error.hpp reaches into daemon/ and cli/ headers), so
# everything is compiled into one target to avoid static-archive link-order
# issues. It exposes the daemon, database (db) and decay subcommands.
add_executable(priorisea
  # CLI commands
  src/cli/main.cpp
  src/cli/util.cpp
  src/cli/commands/daemon.cpp
  src/cli/commands/database.cpp
  src/cli/commands/decay.cpp
  src/cli/commands/formula.cpp
  src/cli/commands/job.cpp
  src/cli/commands/plugin.cpp
  src/cli/commands/timewin.cpp
  src/cli/commands/user.cpp
  # CLI flux endpoint
  src/cli/endpoint/error.cpp
  src/cli/endpoint/util.cpp
  src/cli/endpoint/flux/decay.cpp
  src/cli/endpoint/flux/formula.cpp
  src/cli/endpoint/flux/job.cpp
  src/cli/endpoint/flux/plugin.cpp
  src/cli/endpoint/flux/timewin.cpp
  src/cli/endpoint/flux/user.cpp
  # Daemon core + flux endpoint
  src/daemon/daemon.cpp
  src/daemon/endpoint/error.cpp
  src/daemon/endpoint/flux/endpoint.cpp
  src/daemon/endpoint/flux/callback/decay.cpp
  src/daemon/endpoint/flux/callback/formula.cpp
  src/daemon/endpoint/flux/callback/job.cpp
  src/daemon/endpoint/flux/callback/plugin.cpp
  src/daemon/endpoint/flux/callback/timewin.cpp
  src/daemon/endpoint/flux/callback/user.cpp
  # Database core + sqlite endpoint
  src/database/decay.cpp
  src/database/error.cpp
  src/database/formula.cpp
  src/database/info.cpp
  src/database/sqlite/decay.cpp
  src/database/sqlite/formula.cpp
  src/database/sqlite/info.cpp
  src/database/sqlite/job.cpp
  src/database/sqlite/user.cpp
  src/database/sqlite/sqlite.cpp
  src/database/sqlite/timewin.cpp
  src/database/sqlite/user.cpp
  # Shared library core
  src/lib/decay.cpp
  src/lib/error.cpp
  src/lib/formula.cpp
  src/lib/job.cpp
  src/lib/timewin.cpp
  src/lib/user.cpp
  src/lib/util.cpp)

target_include_directories(priorisea PRIVATE
  ${PRIORISEA_INCLUDE_ROOTS}
  ${PRIORISEA_EXT_INCLUDES})

target_link_libraries(priorisea PRIVATE
  CLI11::CLI11
  PkgConfig::JANSSON
  PkgConfig::SEANERBUS
  PkgConfig::FLUX_CORE
  PkgConfig::SQLITE3
  tinyexpr
  Threads::Threads)

priorisea_add_pch(priorisea)

install(TARGETS priorisea RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Shell completion (only staged when present in the new tree).
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/cli/priorisea.bash)
  install(FILES src/cli/priorisea.bash
    DESTINATION ${CMAKE_INSTALL_DATADIR}/bash-completion/completions
    RENAME priorisea)
endif()

# ---- flux jobtap plugin: sea_priority -----------------------------------
# Loadable module for the flux job-manager. Emitted at
#   {build}/lib/flux/job-manager/plugins/sea_priority.so
# so flux discovers it via its default plugin search path. Compiled as a MODULE
# (dlopen'd, not linked into anything) which also yields the bare `sea_priority`
# name (no `lib` prefix) flux expects.
#
# The plugin reaches into the priorisea library core, so the TUs it pulls in
# (formula/error/util + the error.hpp translation units) are compiled directly
# into the module to avoid a separate linkable priorisea_lib target.
add_library(sea_priority MODULE
  # plugin entry point + jobtap handlers
  src/plugin/flux/sea_priority.cpp
  # priorisea library pieces referenced by the plugin
  src/lib/formula.cpp
  src/lib/error.cpp
  src/lib/util.cpp
  src/cli/endpoint/error.cpp
  src/daemon/endpoint/error.cpp
  src/database/error.cpp)

target_include_directories(sea_priority PRIVATE
  ${PRIORISEA_INCLUDE_ROOTS}
  ${PRIORISEA_EXT_INCLUDES})

target_link_libraries(sea_priority PRIVATE
  PkgConfig::JANSSON
  PkgConfig::FLUX_CORE
  PkgConfig::SQLITE3
  tinyexpr)

priorisea_add_pch(sea_priority)

set_target_properties(sea_priority PROPERTIES
  PREFIX ""
  LIBRARY_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/lib/flux/job-manager/plugins)

# Installed into flux's job-manager plugin dir so `flux jobtap load` / the
# daemon auto-discovers it. Pinned to a literal `lib` (not CMAKE_INSTALL_LIBDIR,
# which resolves to lib64 on Rocky) to match flux-core's own install layout.
install(TARGETS sea_priority LIBRARY DESTINATION lib/flux/job-manager/plugins)

# ---- Unit tests (Catch2) -------------------------------------------------
# Re-enabled once the library is split out into a linkable target again.
#
# add_library(catchmain STATIC src/lib/test/main.cpp)
# target_include_directories(catchmain PUBLIC
#   ${PRIORISEA_INCLUDE_ROOTS} ${PRIORISEA_EXT_INCLUDES} ${CATCH2_INCLUDE_DIR})
# target_link_libraries(catchmain PUBLIC priorisea_lib Catch2::Catch2)
#
# foreach(topic config decay formula score timewin util)
#   add_executable(${topic}_test src/lib/test/${topic}.cpp)
#   target_link_libraries(${topic}_test PRIVATE
#     catchmain priorisea_lib tinyexpr
#     PkgConfig::JANSSON PkgConfig::SEANERBUS PkgConfig::FLUX_CORE
#     Catch2::Catch2)
#   add_test(NAME ${topic} COMMAND ${topic}_test)
# endforeach()

# ---- Integration tests (sharness) ----------------------------------------
# Re-enabled once the jobtap plugins and sharness harness are restored.
#
# file(GLOB SHARNESS_TESTS CONFIGURE_DEPENDS
#   ${CMAKE_CURRENT_SOURCE_DIR}/tests/t[0-9]*.t)
# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/sharness.sh)
#   foreach(t ${SHARNESS_TESTS})
#     get_filename_component(tname ${t} NAME)
#     add_test(NAME ${tname}
#       COMMAND sh ${tname}
#       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
#     set_tests_properties(${tname} PROPERTIES ENVIRONMENT
#       "SHARNESS_BUILD_DIRECTORY=${CMAKE_BINARY_DIR};SHARNESS_TEST_SRCDIR=${CMAKE_CURRENT_SOURCE_DIR}/tests;PATH=${CMAKE_CURRENT_BINARY_DIR}/src/cli:$ENV{PATH}")
#   endforeach()
# endif()

# ---- Docs (Doxygen + Sphinx) --------------------------------------------
# Mirrors the old `make docs` target: generate Doxygen XML for the src/lib
# API and render the Sphinx HTML docs. Only defined when both tools exist so
# it never blocks a plain configure/build.
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

# ---- Code formatting -----------------------------------------------------
# Mirrors the old `make format` target (clang-format -i over src and
# tests/scripts).
find_program(CLANG_FORMAT_EXECUTABLE clang-format)

if(CLANG_FORMAT_EXECUTABLE)
  file(GLOB_RECURSE PRIORISEA_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/scripts/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/scripts/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/scripts/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/scripts/*.hpp)
  add_custom_target(format
    COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${PRIORISEA_SOURCES}
    COMMENT "Formatting sources with clang-format")
endif()
