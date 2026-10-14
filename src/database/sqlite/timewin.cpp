// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/database/sqlite/timewin.hpp"
#include "src/database/error.hpp"
#include "src/database/sqlite/sqlite.hpp"

ps_result_t<ps_timewin_t> ps_db_sqlite_timewin_get(sqlite3 *db,
                                                   const std::string &name) {
  auto get_res = ps_db_sqlite_timewins_get(db, {name});
  if (!ps_is_ok(get_res)) {
    return ps_forward_error<ps_timewin_t>(get_res, "Failed to get timewin");
  }
  auto timewins = ps_unwrap(get_res);
  auto it = timewins.find(name);
  if (it == timewins.end()) {
    // ps_db_sqlite_timewins_get returns an entry for every requested name
    return ps_error(ps_db_not_found_t{.key = "timewin." + name});
  }
  return it->second;
}

ps_result_t<ps_db_timewins_get_t>
ps_db_sqlite_timewins_get(sqlite3 *db, const std::vector<std::string> &names) {
  std::string placeholders;
  for (size_t i = 0; i < names.size(); ++i) {
    if (i > 0)
      placeholders += ", ";
    placeholders += "?";
  }
  auto sql = "SELECT name, cron, duration, score "
             "FROM timewins "
             "WHERE name IN (" +
             placeholders + ");";

  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }

  // Bind the names
  for (auto i = 0; i < names.size(); i++) {
    if (sqlite3_bind_text(stmt, i + 1, names[i].c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK) {
      sqlite3_finalize(stmt);
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
  }

  ps_db_timewins_get_t res;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    auto name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
    res[name] = ps_timewin_t{
        .name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)),
        .cron = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)),
        .duration = sqlite3_column_int(stmt, 2),
        .score = sqlite3_column_double(stmt, 3),
    };
  }
  sqlite3_finalize(stmt);

  // Add errors for timewins that were not found
  for (const auto &name : names) {
    if (res.find(name) == res.end()) {
      res[name] = ps_error(ps_db_not_found_t{.key = "timewin." + name});
    }
  }

  return res;
}

ps_result_t<std::vector<ps_timewin_t>> ps_db_sqlite_timewins_list(sqlite3 *db) {
  auto sql = "SELECT name, cron, duration, score FROM timewins;";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }

  std::vector<ps_timewin_t> res;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    res.push_back(ps_timewin_t{
        .name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)),
        .cron = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)),
        .duration = sqlite3_column_int(stmt, 2),
        .score = sqlite3_column_double(stmt, 3),
    });
  }
  sqlite3_finalize(stmt);
  return res;
}

ps_result_t<ps_success_t>
ps_db_sqlite_timewins_set(sqlite3 *db,
                          const std::vector<ps_timewin_t> &timewins) {
  if (timewins.empty()) {
    return ps_success_t{};
  }

  auto rollback = [&] {
    sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
  };

  if (sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = "BEGIN", .message = sqlite3_errmsg(db)});
  }

  auto sql = "INSERT OR REPLACE INTO timewins (name, cron, duration, score) "
             "VALUES (?, ?, ?, ?);";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  for (const auto &timewin : timewins) {
    if (sqlite3_bind_text(stmt, 1, timewin.name.c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 2, timewin.cron.c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK ||
        sqlite3_bind_int(stmt, 3, timewin.duration) != SQLITE_OK ||
        sqlite3_bind_double(stmt, 4, timewin.score) != SQLITE_OK) {
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

  if (sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = "COMMIT", .message = sqlite3_errmsg(db)});
  }

  return ps_success_t{};
}

ps_result_t<ps_success_t>
ps_db_sqlite_timewin_set(sqlite3 *db, const ps_timewin_t &timewin) {
  return ps_db_sqlite_timewins_set(db, {timewin});
}

ps_result_t<ps_success_t>
ps_db_sqlite_timewins_insert(sqlite3 *db,
                             const std::vector<ps_timewin_t> &timewins) {
  if (timewins.empty()) {
    return ps_success_t{};
  }

  auto rollback = [&] {
    sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
  };

  if (sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = "BEGIN", .message = sqlite3_errmsg(db)});
  }

  auto sql = "INSERT INTO timewins (name, cron, duration, score) "
             "VALUES (?, ?, ?, ?);";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  for (const auto &timewin : timewins) {
    if (sqlite3_bind_text(stmt, 1, timewin.name.c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 2, timewin.cron.c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK ||
        sqlite3_bind_int(stmt, 3, timewin.duration) != SQLITE_OK ||
        sqlite3_bind_double(stmt, 4, timewin.score) != SQLITE_OK) {
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      sqlite3_finalize(stmt);
      rollback();
      if (ps_db_sqlite_is_constraint(db)) {
        return ps_error(
            ps_db_already_exist_err_t{.key = "timewin." + timewin.name});
      }
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
  }
  sqlite3_finalize(stmt);

  if (sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = "COMMIT", .message = sqlite3_errmsg(db)});
  }

  return ps_success_t{};
}

ps_result_t<ps_success_t>
ps_db_sqlite_timewin_insert(sqlite3 *db, const ps_timewin_t &timewin) {
  return ps_db_sqlite_timewins_insert(db, {timewin});
}

ps_result_t<ps_success_t>
ps_db_sqlite_timewins_update(sqlite3 *db,
                             const std::vector<ps_timewin_t> &timewins) {
  if (timewins.empty()) {
    return ps_success_t{};
  }

  auto rollback = [&] {
    sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
  };

  if (sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = "BEGIN", .message = sqlite3_errmsg(db)});
  }

  auto sql = "UPDATE timewins SET cron = ?, duration = ?, score = ? "
             "WHERE name = ?;";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  for (const auto &timewin : timewins) {
    if (sqlite3_bind_text(stmt, 1, timewin.cron.c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK ||
        sqlite3_bind_int(stmt, 2, timewin.duration) != SQLITE_OK ||
        sqlite3_bind_double(stmt, 3, timewin.score) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 4, timewin.name.c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK) {
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
    if (sqlite3_changes(db) == 0) {
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(ps_db_not_found_t{.key = "timewin." + timewin.name});
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
  }
  sqlite3_finalize(stmt);

  if (sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = "COMMIT", .message = sqlite3_errmsg(db)});
  }

  return ps_success_t{};
}

ps_result_t<ps_success_t>
ps_db_sqlite_timewin_update(sqlite3 *db, const ps_timewin_t &timewin) {
  return ps_db_sqlite_timewins_update(db, {timewin});
}

ps_result_t<ps_success_t> ps_db_sqlite_timewin_del(sqlite3 *db,
                                                   const std::string &name) {
  return ps_db_sqlite_timewins_del(db, {name});
}

ps_result_t<ps_success_t>
ps_db_sqlite_timewins_del(sqlite3 *db, const std::vector<std::string> &names) {
  // Deletion is all-or-none: a timewin is only deleted if every requested
  // timewin exists, so we first check that they all exist.
  auto get_res = ps_db_sqlite_timewins_get(db, names);
  if (!ps_is_ok(get_res)) {
    return ps_forward_error<ps_success_t>(
        get_res, "Failed to retrieve timewins before deletion");
  }

  auto get_val = ps_unwrap(get_res);
  for (const auto &name : names) {
    auto it = get_val.find(name);
    if (it == get_val.end()) {
      // ps_db_sqlite_timewins_get is supposed to return a value for each name
      return ps_error(ps_unexpected_err_t{
          .msg =
              "ps_db_sqlite_timewins_get did not return an entry for timewin " +
              name});
    }
    if (!ps_is_ok(it->second)) {
      // The timewin does not exist: delete nothing
      return ps_error(ps_db_not_found_t{.key = "timewin." + name});
    }
  }

  if (names.empty()) {
    return ps_success_t{};
  }

  auto rollback = [&] {
    sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
  };

  if (sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = "BEGIN", .message = sqlite3_errmsg(db)});
  }

  std::string placeholders;
  for (size_t i = 0; i < names.size(); ++i) {
    if (i > 0)
      placeholders += ", ";
    placeholders += "?";
  }
  auto sql = "DELETE FROM timewins WHERE name IN (" + placeholders + ");";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }

  // Bind the names
  for (size_t i = 0; i < names.size(); i++) {
    if (sqlite3_bind_text(stmt, i + 1, names[i].c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK) {
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
  }

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    sqlite3_finalize(stmt);
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  sqlite3_finalize(stmt);

  if (sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = "COMMIT", .message = sqlite3_errmsg(db)});
  }

  return ps_success_t{};
}
