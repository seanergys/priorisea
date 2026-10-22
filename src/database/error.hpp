// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <flux/core.h>
#include <string>
#include <sys/types.h>

/**
 * A requested database resource was not found
 */
struct ps_db_not_found_t {
  std::string key;
};

/**
 * A database resource already exists
 */
struct ps_db_already_exist_err_t {
  std::string key;
};

/**
 * The database could not be opened
 */
struct ps_db_open_err_t {
  std::string path;
  std::string message;
};

/**
 * An SQL query failed to execute
 */
struct ps_db_query_err_t {
  std::string query;
  std::string message;
};

/**
 * The database schema version does not match the expected one
 */
struct ps_db_version_mismatch_t {
  int expected;
  int actual;
};

/**
 * Renders each database error kind into a human readable message
 */
std::string ps_error_message(const ps_db_not_found_t &err);
std::string ps_error_message(const ps_db_already_exist_err_t &err);
std::string ps_error_message(const ps_db_open_err_t &err);
std::string ps_error_message(const ps_db_query_err_t &err);
std::string ps_error_message(const ps_db_version_mismatch_t &err);
