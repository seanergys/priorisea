// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/database/sqlite/decay.hpp"
#include "src/database/error.hpp"
#include "src/database/sqlite/sqlite.hpp"
#include "src/lib/error.hpp"

ps_result_t<ps_decay_t> ps_db_sqlite_decay_get(sqlite3 *db) {
  // Order is ascending
  auto sql = "SELECT duration, weight "
             "FROM decay_items "
             "ORDER BY order_id ASC;";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }

  ps_decay_t res;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    res.push_back(ps_decay_frame_t{
        .duration = sqlite3_column_int(stmt, 0),
        .weight = sqlite3_column_double(stmt, 1),
    });
  }

  if (res.size() == 0) {
    return ps_error(ps_db_not_found_t{.key = "decay"});
  }

  sqlite3_finalize(stmt);
  return res;
}

ps_result_t<ps_success_t> ps_db_sqlite_decay_set(sqlite3 *db,
                                                 const ps_decay_t &decay) {

  auto rollback = [&] {
    sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
  };

  if (sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = "BEGIN", .message = sqlite3_errmsg(db)});
  }

  // Rollback the delete in case something goes wrong with the insert
  if (sqlite3_exec(db, "DELETE FROM decay_items", nullptr, nullptr, nullptr) !=
      SQLITE_OK) {
    rollback();
    return ps_error(ps_db_query_err_t{.query = "DELETE FROM decay_items",
                                      .message = sqlite3_errmsg(db)});
  }

  auto sql = "INSERT INTO decay_items (order_id, duration, weight) "
             "VALUES (?, ?, ?);";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  // Order is ascending
  for (size_t i = 0; i < decay.size(); ++i) {
    // Bind the parameters
    if (sqlite3_bind_int(stmt, 1, static_cast<int>(i)) != SQLITE_OK ||
        sqlite3_bind_int(stmt, 2, decay[i].duration) != SQLITE_OK ||
        sqlite3_bind_double(stmt, 3, decay[i].weight) != SQLITE_OK) {
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
  }
  sqlite3_finalize(stmt);

  // Commit the transaction
  if (sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = "COMMIT", .message = sqlite3_errmsg(db)});
  }

  return ps_success_t{};
}

ps_result_t<ps_success_t> ps_db_sqlite_decay_insert(sqlite3 *db,
                                                    const ps_decay_t &decay) {
  auto rollback = [&] {
    sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
  };

  if (sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = "BEGIN", .message = sqlite3_errmsg(db)});
  }

  // Fail if a decay config already exists
  auto count_sql = "SELECT COUNT(*) FROM decay_items;";
  sqlite3_stmt *count_stmt = nullptr;
  if (sqlite3_prepare_v2(db, count_sql, -1, &count_stmt, nullptr) !=
      SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = count_sql, .message = sqlite3_errmsg(db)});
  }
  if (sqlite3_step(count_stmt) != SQLITE_ROW) {
    sqlite3_finalize(count_stmt);
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = count_sql, .message = sqlite3_errmsg(db)});
  }
  auto count = sqlite3_column_int(count_stmt, 0);
  sqlite3_finalize(count_stmt);
  if (count > 0) {
    rollback();
    return ps_error(ps_db_already_exist_err_t{.key = "decay"});
  }

  // Order is ascending
  auto sql = "INSERT INTO decay_items (order_id, duration, weight) "
             "VALUES (?, ?, ?);";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  for (size_t i = 0; i < decay.size(); ++i) {
    // Bind the parameters
    if (sqlite3_bind_int(stmt, 1, static_cast<int>(i)) != SQLITE_OK ||
        sqlite3_bind_int(stmt, 2, decay[i].duration) != SQLITE_OK ||
        sqlite3_bind_double(stmt, 3, decay[i].weight) != SQLITE_OK) {
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      sqlite3_finalize(stmt);
      rollback();
      if (ps_db_sqlite_is_constraint(db)) {
        return ps_error(ps_db_already_exist_err_t{.key = "decay"});
      }
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
  }
  sqlite3_finalize(stmt);

  // Commit the transaction
  if (sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = "COMMIT", .message = sqlite3_errmsg(db)});
  }

  return ps_success_t{};
}

ps_result_t<ps_success_t> ps_db_sqlite_decay_update(sqlite3 *db,
                                                    const ps_decay_t &decay) {
  // Fail if no decay config exists yet
  auto count_sql = "SELECT COUNT(*) FROM decay_items;";
  sqlite3_stmt *count_stmt = nullptr;
  if (sqlite3_prepare_v2(db, count_sql, -1, &count_stmt, nullptr) !=
      SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = count_sql, .message = sqlite3_errmsg(db)});
  }
  if (sqlite3_step(count_stmt) != SQLITE_ROW) {
    sqlite3_finalize(count_stmt);
    return ps_error(
        ps_db_query_err_t{.query = count_sql, .message = sqlite3_errmsg(db)});
  }
  auto count = sqlite3_column_int(count_stmt, 0);
  sqlite3_finalize(count_stmt);
  if (count == 0) {
    return ps_error(ps_db_not_found_t{.key = "decay"});
  }

  auto rollback = [&] {
    sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
  };

  if (sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = "BEGIN", .message = sqlite3_errmsg(db)});
  }

  // Replace the existing frames
  if (sqlite3_exec(db, "DELETE FROM decay_items", nullptr, nullptr, nullptr) !=
      SQLITE_OK) {
    rollback();
    return ps_error(ps_db_query_err_t{.query = "DELETE FROM decay_items",
                                      .message = sqlite3_errmsg(db)});
  }

  // Order is ascending
  auto sql = "INSERT INTO decay_items (order_id, duration, weight) "
             "VALUES (?, ?, ?);";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  for (size_t i = 0; i < decay.size(); ++i) {
    // Bind the parameters
    if (sqlite3_bind_int(stmt, 1, static_cast<int>(i)) != SQLITE_OK ||
        sqlite3_bind_int(stmt, 2, decay[i].duration) != SQLITE_OK ||
        sqlite3_bind_double(stmt, 3, decay[i].weight) != SQLITE_OK) {
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
  }
  sqlite3_finalize(stmt);

  // Commit the transaction
  if (sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = "COMMIT", .message = sqlite3_errmsg(db)});
  }

  return ps_success_t{};
}
