#include "src/database/sqlite/formula.hpp"
#include "src/database/error.hpp"
#include "src/database/sqlite/sqlite.hpp"
#include "src/lib/error.hpp"
#include "src/lib/formula.hpp"
#include "src/lib/types.hpp"

ps_result_t<ps_formula_t> ps_db_sqlite_formula_get(sqlite3 *db,
                                                   const std::string &name) {
  auto get_res = ps_db_sqlite_formulas_get(db, {name});
  if (!ps_is_ok(get_res)) {
    return ps_forward_error<ps_formula_t>(get_res, "Failed to get formula");
  }
  auto formulas = ps_unwrap(get_res);
  auto it = formulas.find(name);
  if (it == formulas.end()) {
    // ps_db_sqlite_formulas_get returns an entry for every requested name
    return ps_error(ps_db_not_found_t{.key = "formula." + name});
  }
  return it->second;
}

ps_result_t<ps_db_formulas_get_t>
ps_db_sqlite_formulas_get(sqlite3 *db, const std::vector<std::string> &names) {
  std::string placeholders;
  for (size_t i = 0; i < names.size(); ++i) {
    if (i > 0)
      placeholders += ", ";
    placeholders += "?";
  }
  // The defaults map is stored as a JSON object in a single column.
  auto sql = "SELECT name, expression, f_defaults "
             "FROM formulas "
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

  ps_db_formulas_get_t res;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    auto name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
    auto expr = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
    auto defaults_str =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
    auto defaults_res = ps_formula_vars_from_json_str(defaults_str);
    if (!ps_is_ok(defaults_res)) {
      sqlite3_finalize(stmt);
      return ps_forward_error<ps_db_formulas_get_t>(defaults_res);
    }
    res[name] = ps_formula_t{
        .name = name, .expr = expr, .defaults = ps_unwrap(defaults_res)};
  }

  // Add errors for formulas that were not found
  for (const auto &name : names) {
    if (res.find(name) == res.end()) {
      res[name] = ps_error(ps_db_not_found_t{.key = "formula." + name});
    }
  }

  return res;
}

ps_result_t<std::vector<ps_formula_t>> ps_db_sqlite_formulas_list(sqlite3 *db) {
  auto sql = "SELECT name, expression, f_defaults FROM formulas;";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }

  std::map<std::string, ps_formula_t> formulas;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    auto name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
    auto expr = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
    auto defaults_str =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
    auto defaults_res = ps_formula_vars_from_json_str(defaults_str);
    if (!ps_is_ok(defaults_res)) {
      sqlite3_finalize(stmt);
      return ps_forward_error<std::vector<ps_formula_t>>(defaults_res);
    }
    formulas[name] = ps_formula_t{
        .name = name, .expr = expr, .defaults = ps_unwrap(defaults_res)};
  }
  sqlite3_finalize(stmt);

  // Convert the map to a vector
  std::vector<ps_formula_t> res;
  for (const auto &[name, formula] : formulas) {
    res.push_back(formula);
  }
  return res;
}

ps_result_t<ps_success_t>
ps_db_sqlite_formulas_set(sqlite3 *db,
                          const std::vector<ps_formula_t> &formulas) {
  if (formulas.empty()) {
    return ps_success_t{};
  }

  // If a transaction is already open, let the caller own it and skip our own
  // BEGIN/COMMIT/ROLLBACK.
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

  // The expression and the defaults map are stored together in the formulas
  // row; the defaults map is serialized as a JSON object.
  auto sql = "INSERT OR REPLACE INTO formulas (name, expression, f_defaults) "
             "VALUES (?, ?, ?);";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }

  for (const auto &formula : formulas) {
    auto defaults_json = ps_formula_vars_to_json_str(formula.defaults);
    if (sqlite3_bind_text(stmt, 1, formula.name.c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 2, formula.expr.c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 3, defaults_json.c_str(), -1,
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

ps_result_t<ps_success_t>
ps_db_sqlite_formula_set(sqlite3 *db, const ps_formula_t &formula) {
  return ps_db_sqlite_formulas_set(db, {formula});
}

ps_result_t<ps_success_t>
ps_db_sqlite_formulas_insert(sqlite3 *db,
                             const std::vector<ps_formula_t> &formulas) {
  if (formulas.empty()) {
    return ps_success_t{};
  }

  // If a transaction is already open, let the caller own it and skip our own
  // BEGIN/COMMIT/ROLLBACK.
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

  // Add to formulas table. A plain insert fails if the formula already exists.
  auto sql = "INSERT INTO formulas (name, expression, f_defaults) "
             "VALUES (?, ?, ?);";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }

  for (const auto &formula : formulas) {
    auto defaults_json = ps_formula_vars_to_json_str(formula.defaults);
    if (sqlite3_bind_text(stmt, 1, formula.name.c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 2, formula.expr.c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 3, defaults_json.c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK) {
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
            ps_db_already_exist_err_t{.key = "formula." + formula.name});
      }
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

ps_result_t<ps_success_t>
ps_db_sqlite_formula_insert(sqlite3 *db, const ps_formula_t &formula) {
  return ps_db_sqlite_formulas_insert(db, {formula});
}

ps_result_t<ps_success_t>
ps_db_sqlite_formulas_update(sqlite3 *db,
                             const std::vector<ps_formula_t> &formulas) {
  if (formulas.empty()) {
    return ps_success_t{};
  }

  // If a transaction is already open, let the caller own it and skip our own
  // BEGIN/COMMIT/ROLLBACK.
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

  // Update the formulas row. If no row is affected the formula is absent
  // The defaults map is stored alongside the expression as a JSON object
  auto sql =
      "UPDATE formulas SET expression = ?, f_defaults = ? WHERE name = ?;";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }

  for (const auto &formula : formulas) {
    auto defaults_json = ps_formula_vars_to_json_str(formula.defaults);
    if (sqlite3_bind_text(stmt, 1, formula.expr.c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 2, defaults_json.c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 3, formula.name.c_str(), -1,
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
      return ps_error(ps_db_not_found_t{.key = "formula." + formula.name});
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

ps_result_t<ps_success_t>
ps_db_sqlite_formula_update(sqlite3 *db, const ps_formula_t &formula) {
  return ps_db_sqlite_formulas_update(db, {formula});
}
