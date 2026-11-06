#include "src/database/sqlite/user.hpp"
#include "src/database/error.hpp"
#include "src/database/sqlite/sqlite.hpp"
#include "src/lib/formula.hpp"
#include "src/lib/util.hpp"
#include <string>
#include <sys/types.h>

ps_result_t<ps_user_t> ps_db_sqlite_user_get(sqlite3 *db, uid_t userid) {
  auto get_res = ps_db_sqlite_users_get(db, {userid});
  if (!ps_is_ok(get_res)) {
    return ps_forward_error<ps_user_t>(get_res, "Failed to get user");
  }
  auto users = ps_unwrap(get_res);
  auto it = users.find(userid);
  if (it == users.end()) {
    // ps_db_sqlite_users_get returns an entry for every requested userid
    return ps_error(ps_db_not_found_t{.key = "user." + std::to_string(userid)});
  }
  return it->second;
}

ps_result_t<ps_db_sqlite_users_get_t>
ps_db_sqlite_users_get(sqlite3 *db, const std::vector<uid_t> &userids) {
  std::string placeholders;
  for (size_t i = 0; i < userids.size(); ++i) {
    if (i > 0)
      placeholders += ", ";
    placeholders += "?";
  }
  auto sql = "SELECT userid, score, w_expr, w_defaults, t_executed, "
             "t_inactive_min "
             "FROM users "
             "WHERE userid IN (" +
             placeholders + ");";

  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }

  // Bind the userids
  for (auto i = 0; i < userids.size(); i++) {
    if (sqlite3_bind_int(stmt, i + 1, userids[i]) != SQLITE_OK) {
      sqlite3_finalize(stmt);
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
  }

  ps_db_sqlite_users_get_t res;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    uid_t userid = static_cast<uid_t>(sqlite3_column_int(stmt, 0));
    std::optional<double> score;
    if (sqlite3_column_type(stmt, 1) != SQLITE_NULL) {
      score = sqlite3_column_double(stmt, 1);
    }
    auto w_expr = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
    auto w_defaults_str =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
    auto w_defaults_res = ps_formula_vars_from_json_str(w_defaults_str);
    if (!ps_is_ok(w_defaults_res)) {
      sqlite3_finalize(stmt);
      return ps_forward_error<ps_db_sqlite_users_get_t>(w_defaults_res);
    }
    res[userid] = ps_user_t{
        .userid = userid,
        .score = score,
        .w_expr = w_expr,
        .w_defaults = ps_unwrap(w_defaults_res),
        .t_executed = sqlite3_column_double(stmt, 4),
        .t_inactive_min = sqlite3_column_double(stmt, 5),
    };
  }
  sqlite3_finalize(stmt);

  // Add errors for users that were not found
  for (const auto &userid : userids) {
    if (res.find(userid) == res.end()) {
      res[userid] =
          ps_error(ps_db_not_found_t{.key = "user." + std::to_string(userid)});
    }
  }

  return res;
}

ps_result_t<std::vector<ps_user_t>>
ps_db_sqlite_users_list(sqlite3 *db, ps_db_order_t order) {
  // Order by the user id; only the direction is chosen by the caller.
  auto sql = "SELECT userid, score, w_expr, w_defaults, t_executed, "
             "t_inactive_min FROM users ORDER BY userid " +
             ps_db_order_to_str(order) + ";";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }

  std::vector<ps_user_t> res;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    uid_t userid = static_cast<uid_t>(sqlite3_column_int(stmt, 0));
    std::optional<double> score;
    if (sqlite3_column_type(stmt, 1) != SQLITE_NULL) {
      score = sqlite3_column_double(stmt, 1);
    }
    auto w_expr = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
    auto w_defaults_str =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
    auto w_defaults_res = ps_formula_vars_from_json_str(w_defaults_str);
    if (!ps_is_ok(w_defaults_res)) {
      sqlite3_finalize(stmt);
      return ps_forward_error<std::vector<ps_user_t>>(w_defaults_res);
    }
    res.push_back(ps_user_t{
        .userid = userid,
        .score = score,
        .w_expr = w_expr,
        .w_defaults = ps_unwrap(w_defaults_res),
        .t_executed = sqlite3_column_double(stmt, 4),
        .t_inactive_min = sqlite3_column_double(stmt, 5),
    });
  }
  sqlite3_finalize(stmt);
  return res;
}

ps_result_t<ps_success_t>
ps_db_sqlite_users_set(sqlite3 *db, const std::vector<ps_user_t> &users) {
  if (users.empty()) {
    return ps_success_t{};
  }

  // If a transaction is already open, let the caller own it and skip our own
  // BEGIN/COMMIT/ROLLBACK, so the set can be combined with other writes in a
  // single transaction.
  bool outer_txn = sqlite3_get_autocommit(db) == 0;

  auto rollback = [&] {
    if (!outer_txn)
      sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
  };

  if (!outer_txn) {
    if (sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr) != SQLITE_OK) {
      return ps_error(
          ps_db_query_err_t{.query = "BEGIN", .message = sqlite3_errmsg(db)});
    }
  }

  auto sql = "INSERT OR REPLACE INTO users (userid, score, w_expr, "
             "w_defaults, t_executed, t_inactive_min) "
             "VALUES (?, ?, ?, ?, ?, ?);";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  for (const auto &user : users) {
    auto w_defaults_json = ps_formula_vars_to_json_str(user.w_defaults);
    if (sqlite3_bind_int(stmt, 1, user.userid) != SQLITE_OK ||
        (user.score ? sqlite3_bind_double(stmt, 2, *user.score)
                    : sqlite3_bind_null(stmt, 2)) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 3, user.w_expr.c_str(), -1, SQLITE_TRANSIENT) !=
            SQLITE_OK ||
        sqlite3_bind_text(stmt, 4, w_defaults_json.c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK ||
        sqlite3_bind_double(stmt, 5, user.t_executed) != SQLITE_OK ||
        sqlite3_bind_double(stmt, 6, user.t_inactive_min) != SQLITE_OK) {
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

  if (!outer_txn) {
    if (sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr) != SQLITE_OK) {
      rollback();
      return ps_error(
          ps_db_query_err_t{.query = "COMMIT", .message = sqlite3_errmsg(db)});
    }
  }

  return ps_success_t{};
}

ps_result_t<ps_success_t> ps_db_sqlite_user_set(sqlite3 *db,
                                                const ps_user_t &user) {
  return ps_db_sqlite_users_set(db, {user});
}

ps_result_t<ps_success_t>
ps_db_sqlite_users_insert(sqlite3 *db, const std::vector<ps_user_t> &users) {
  if (users.empty()) {
    return ps_success_t{};
  }

  auto rollback = [&] {
    sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
  };

  if (sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = "BEGIN", .message = sqlite3_errmsg(db)});
  }

  auto sql = "INSERT INTO users (userid, score, w_expr, w_defaults, "
             "t_executed, t_inactive_min) VALUES (?, ?, ?, ?, ?, ?);";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  for (const auto &user : users) {
    auto w_defaults_json = ps_formula_vars_to_json_str(user.w_defaults);
    if (sqlite3_bind_int(stmt, 1, user.userid) != SQLITE_OK ||
        (user.score ? sqlite3_bind_double(stmt, 2, *user.score)
                    : sqlite3_bind_null(stmt, 2)) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 3, user.w_expr.c_str(), -1, SQLITE_TRANSIENT) !=
            SQLITE_OK ||
        sqlite3_bind_text(stmt, 4, w_defaults_json.c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK ||
        sqlite3_bind_double(stmt, 5, user.t_executed) != SQLITE_OK ||
        sqlite3_bind_double(stmt, 6, user.t_inactive_min) != SQLITE_OK) {
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      sqlite3_finalize(stmt);
      rollback();
      if (ps_db_sqlite_is_constraint(db)) {
        return ps_error(ps_db_already_exist_err_t{
            .key = "user." + std::to_string(user.userid)});
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

ps_result_t<ps_success_t> ps_db_sqlite_user_insert(sqlite3 *db,
                                                   const ps_user_t &user) {
  return ps_db_sqlite_users_insert(db, {user});
}

ps_result_t<ps_success_t>
ps_db_sqlite_users_update(sqlite3 *db, const std::vector<ps_user_t> &users) {
  if (users.empty()) {
    return ps_success_t{};
  }

  auto rollback = [&] {
    sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
  };

  if (sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = "BEGIN", .message = sqlite3_errmsg(db)});
  }

  auto sql = "UPDATE users SET score = ?, w_expr = ?, w_defaults = ?, "
             "t_executed = ?, t_inactive_min = ? WHERE userid = ?;";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  for (const auto &user : users) {
    auto w_defaults_json = ps_formula_vars_to_json_str(user.w_defaults);
    if ((user.score ? sqlite3_bind_double(stmt, 1, *user.score)
                    : sqlite3_bind_null(stmt, 1)) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 2, user.w_expr.c_str(), -1, SQLITE_TRANSIENT) !=
            SQLITE_OK ||
        sqlite3_bind_text(stmt, 3, w_defaults_json.c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK ||
        sqlite3_bind_double(stmt, 4, user.t_executed) != SQLITE_OK ||
        sqlite3_bind_double(stmt, 5, user.t_inactive_min) != SQLITE_OK ||
        sqlite3_bind_int(stmt, 6, user.userid) != SQLITE_OK) {
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
      return ps_error(
          ps_db_not_found_t{.key = "user." + std::to_string(user.userid)});
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

ps_result_t<ps_success_t> ps_db_sqlite_user_update(sqlite3 *db,
                                                   const ps_user_t &user) {
  return ps_db_sqlite_users_update(db, {user});
}
