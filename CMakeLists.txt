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
pkg_check_modules(JANSSON   REQUIRED IMPORTED_TARGET jansson)
pkg_check_modules(FMT       REQUIRED IMPORTED_TARGET fmt)
pkg_check_modules(SEANERBUS REQUIRED IMPORTED_TARGET seanerbus-c)
pkg_check_modules(FLUX_CORE REQUIRED IMPORTED_TARGET flux-core>=0.81.0)

# CLI11
find_path(CLI11_INCLUDE_DIR CLI/CLI.hpp REQUIRED)
add_library(CLI11::CLI11 INTERFACE IMPORTED)
set_target_properties(CLI11::CLI11 PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${CLI11_INCLUDE_DIR}")

# Catch2 v2
find_path(CATCH2_INCLUDE_DIR catch2/catch.hpp REQUIRED)
add_library(Catch2::Catch2 INTERFACE IMPORTED)
set_target_properties(Catch2::Catch2 PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${CATCH2_INCLUDE_DIR}")

# ---- External: tinyexpr -------------------------------------------------
add_library(tinyexpr STATIC external/tinyexpr/tinyexpr.c)
target_include_directories(tinyexpr PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/external/tinyexpr)

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

# Common include dirs used by several targets.
set(PRIORISEA_EXT_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/external/tinyexpr
  ${CMAKE_CURRENT_SOURCE_DIR}/external/croncpp/include)

# ---- libpriorisea (noinst, static + PIC) --------------------------------
add_library(priorisea_lib STATIC
  src/lib/config.cpp
  src/lib/decay.cpp
  src/lib/error.cpp
  src/lib/flux.cpp
  src/lib/formula.cpp
  src/lib/score.cpp
  src/lib/timewin.cpp
  src/lib/util.cpp)

target_include_directories(priorisea_lib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lib
  ${PRIORISEA_EXT_INCLUDES})
target_link_libraries(priorisea_lib PUBLIC PkgConfig::JANSSON PkgConfig::FLUX_CORE PkgConfig::SEANERBUS tinyexpr PkgConfig::FMT)
priorisea_add_pch(priorisea_lib)

# ---- priorisea CLI -------------------------------------------------------
add_executable(priorisea
  src/cli/main.cpp
  src/cli/formula.cpp
  src/cli/timewin.cpp
  src/cli/score.cpp
  src/cli/decay.cpp
  src/cli/init.cpp
  src/cli/util.cpp
  src/cli/plugin.cpp
  src/cli/config.cpp)

target_include_directories(priorisea PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cli
  ${PRIORISEA_EXT_INCLUDES})
target_link_libraries(priorisea PRIVATE
  priorisea_lib CLI11::CLI11 PkgConfig::JANSSON PkgConfig::SEANERBUS PkgConfig::FLUX_CORE)
priorisea_add_pch(priorisea)

install(TARGETS priorisea RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES src/cli/priorisea.bash
  DESTINATION ${CMAKE_INSTALL_DATADIR}/bash-completion/completions
  RENAME priorisea)
# The sharness completion test sources
add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/src/cli/priorisea.bash
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_CURRENT_SOURCE_DIR}/src/cli/priorisea.bash
          ${CMAKE_BINARY_DIR}/src/cli/priorisea.bash
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/cli/priorisea.bash
  COMMENT "Staging priorisea.bash into the build tree")
add_custom_target(priorisea_completion_script ALL
  DEPENDS ${CMAKE_BINARY_DIR}/src/cli/priorisea.bash)

# ---- flux jobtap plugins (MODULE, no lib prefix, no version) ------------
function(priorisea_add_jobtap_plugin name)
  add_library(${name} MODULE ${CMAKE_CURRENT_SOURCE_DIR}/src/plugins/${name}.cpp)
  target_include_directories(${name} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src ${PRIORISEA_EXT_INCLUDES})
  # Sharness integration tests (t1000-basic.t etc.) look for the module at
  # ${SHARNESS_BUILD_DIRECTORY}/src/plugins/.libs/<name>.so (the libtool layout).
  set_target_properties(${name} PROPERTIES PREFIX "" OUTPUT_NAME ${name}
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/plugins/.libs")
  install(TARGETS ${name} LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/flux/job-manager/plugins)
endfunction()

priorisea_add_jobtap_plugin(sea_priority)
target_link_libraries(sea_priority PRIVATE
  priorisea_lib PkgConfig::FMT PkgConfig::JANSSON PkgConfig::SEANERBUS PkgConfig::FLUX_CORE)
priorisea_add_pch(sea_priority)

priorisea_add_jobtap_plugin(sea_debug)
target_link_libraries(sea_debug PRIVATE PkgConfig::JANSSON PkgConfig::FLUX_CORE)

# ---- seanerbus example clients (sharness test helpers) ------------------
foreach(sb IN ITEMS sb_response sb_register sb_request sb_resolve)
  add_executable(${sb} ${CMAKE_CURRENT_SOURCE_DIR}/tests/scripts/${sb}.cpp)
  target_link_libraries(${sb} PRIVATE PkgConfig::SEANERBUS)
  set_target_properties(${sb} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/scripts)
endforeach()

# ---- Unit tests (Catch2) -------------------------------------------------
add_library(catchmain STATIC src/lib/test/main.cpp)
target_include_directories(catchmain PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src ${PRIORISEA_EXT_INCLUDES}
  ${CATCH2_INCLUDE_DIR})
target_link_libraries(catchmain PUBLIC priorisea_lib Catch2::Catch2)

foreach(topic config decay formula score timewin util)
  add_executable(${topic}_test src/lib/test/${topic}.cpp)
  target_link_libraries(${topic}_test PRIVATE
    catchmain priorisea_lib tinyexpr PkgConfig::JANSSON PkgConfig::SEANERBUS PkgConfig::FLUX_CORE Catch2::Catch2)
  add_test(NAME ${topic} COMMAND ${topic}_test)
endforeach()

# ---- Integration tests (sharness) ---------------------------------------
file(GLOB SHARNESS_TESTS CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/t[0-9]*.t)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/sharness.sh)
  foreach(t ${SHARNESS_TESTS})
    get_filename_component(tname ${t} NAME)
    add_test(NAME ${tname}
      COMMAND sh ${tname}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    # SHARNESS_BUILD_DIRECTORY is set to the build tree so the hardcoded
    # ${SHARNESS_BUILD_DIRECTORY}/src/plugins/.libs/sea_priority.so resolves
    set_tests_properties(${tname} PROPERTIES ENVIRONMENT
      "SHARNESS_BUILD_DIRECTORY=${CMAKE_BINARY_DIR};SHARNESS_TEST_SRCDIR=${CMAKE_CURRENT_SOURCE_DIR}/tests;PATH=${CMAKE_CURRENT_BINARY_DIR}/src/cli:$ENV{PATH}")
  endforeach()
endif()

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
