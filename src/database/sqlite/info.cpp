// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/database/sqlite/info.hpp"
#include "src/database/error.hpp"
#include "src/lib/error.hpp"

ps_result_t<double> ps_db_sqlite_t_inactive_get(sqlite3 *db) {
  auto sql = "SELECT t_inactive FROM database_info WHERE id = 1;";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  if (sqlite3_step(stmt) != SQLITE_ROW) {
    sqlite3_finalize(stmt);
    return ps_error(ps_db_not_found_t{.key = "t_inactive"});
  }
  double t_inactive = sqlite3_column_double(stmt, 0);
  sqlite3_finalize(stmt);
  return t_inactive;
}

ps_result_t<ps_success_t> ps_db_sqlite_t_inactive_set(sqlite3 *db,
                                                      double t_inactive) {
  auto sql = "INSERT OR REPLACE INTO database_info (id, t_inactive) "
             "VALUES (1, ?);";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  // Bind the value
  if (sqlite3_bind_double(stmt, 1, t_inactive) != SQLITE_OK) {
    sqlite3_finalize(stmt);
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  if (sqlite3_step(stmt) != SQLITE_DONE) {
    sqlite3_finalize(stmt);
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  sqlite3_finalize(stmt);
  return ps_success_t{};
}
