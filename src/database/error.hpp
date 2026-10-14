// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <flux/core.h>
#include <string>
#include <sys/types.h>

struct ps_db_not_found_t {
  std::string key;
};

struct ps_db_already_exist_err_t {
  std::string key;
};

struct ps_db_open_err_t {
  std::string path;
  std::string message;
};

struct ps_db_query_err_t {
  std::string query;
  std::string message;
};

struct ps_db_version_mismatch_t {
  int expected;
  int actual;
};

std::string ps_error_message(const ps_db_not_found_t &err);
std::string ps_error_message(const ps_db_already_exist_err_t &err);
std::string ps_error_message(const ps_db_open_err_t &err);
std::string ps_error_message(const ps_db_query_err_t &err);
std::string ps_error_message(const ps_db_version_mismatch_t &err);
