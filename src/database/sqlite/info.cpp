#include "src/database/sqlite/info.hpp"
#include "src/database/error.hpp"
#include "src/lib/error.hpp"

ps_result_t<ps_db_info_t> ps_db_sqlite_info_get(sqlite3 *db) {
  auto sql = "SELECT t_inactive, user_period FROM database_info WHERE id = 1;";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  if (sqlite3_step(stmt) != SQLITE_ROW) {
    sqlite3_finalize(stmt);
    return ps_error(ps_db_not_found_t{.key = "database_info"});
  }
  auto info = ps_db_info_t{
      .t_inactive = sqlite3_column_double(stmt, 0),
      .user_period = sqlite3_column_double(stmt, 1),
  };
  sqlite3_finalize(stmt);
  return info;
}

ps_result_t<ps_success_t> ps_db_sqlite_info_set(sqlite3 *db,
                                                const ps_db_info_t &info) {
  auto sql = "INSERT OR REPLACE INTO database_info "
             "(id, t_inactive, user_period) VALUES (1, ?, ?);";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  if (sqlite3_bind_double(stmt, 1, info.t_inactive) != SQLITE_OK ||
      sqlite3_bind_double(stmt, 2, info.user_period) != SQLITE_OK) {
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
