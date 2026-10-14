// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/database/error.hpp"

std::string ps_error_message(const ps_db_not_found_t &err) {
  return "Resource " + err.key + " could not be found";
}

std::string ps_error_message(const ps_db_already_exist_err_t &err) {
  return "Resource " + err.key + " already exists";
}

std::string ps_error_message(const ps_db_open_err_t &err) {
  return "Error opening database at: " + err.path + ": " + err.message;
}

std::string ps_error_message(const ps_db_version_mismatch_t &err) {
  return "Database version mismatch: expected " + std::to_string(err.expected) +
         ", got " + std::to_string(err.actual);
}

std::string ps_error_message(const ps_db_query_err_t &err) {
  return "Error executing query: " + err.query + ": " + err.message;
}
