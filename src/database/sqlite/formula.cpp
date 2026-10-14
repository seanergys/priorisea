// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/database/sqlite/formula.hpp"
#include "src/database/error.hpp"
#include "src/database/sqlite/sqlite.hpp"
#include "src/lib/error.hpp"
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
  auto sql = "SELECT f.name, f.expression, d.var, d.value "
             "FROM formulas f JOIN formula_defaults d ON f.name = d.formula "
             "WHERE f.name IN (" +
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
    // Create a new entry if the formula is not already in the map
    if (res.find(name) == res.end()) {
      res[name] = ps_formula_t{
          .name = name,
          .expr = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)),
          .defaults = {}};
    }
    std::string var =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
    auto value = sqlite3_column_double(stmt, 3);
    std::get<ps_formula_t>(res[name]).defaults[var] = value;
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
  auto sql = "SELECT f.name, f.expression, d.var, d.value "
             "FROM formulas f JOIN formula_defaults d ON f.name = d.formula;";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }

  std::map<std::string, ps_formula_t> formulas;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    auto name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
    // Create a new entry if the formula is not already in the map
    if (formulas.find(name) == formulas.end()) {
      formulas[name] = ps_formula_t{
          .name = name,
          .expr = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)),
          .defaults = {}};
    }
    auto var = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
    auto value = sqlite3_column_double(stmt, 3);
    formulas[name].defaults[var] = value;
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

  auto rollback = [&] {
    sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
  };

  if (sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = "BEGIN", .message = sqlite3_errmsg(db)});
  }

  // Add to formulas table
  auto sql =
      "INSERT OR REPLACE INTO formulas (name, expression) VALUES (?, ?);";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }

  // Add to formula_defaults table
  auto sql_defaults = "INSERT OR REPLACE INTO formula_defaults (formula, var, "
                      "value) VALUES (?, ?, ?);";
  sqlite3_stmt *stmt_defaults = nullptr;
  if (sqlite3_prepare_v2(db, sql_defaults, -1, &stmt_defaults, nullptr) !=
      SQLITE_OK) {
    sqlite3_finalize(stmt);
    rollback();
    return ps_error(ps_db_query_err_t{.query = sql_defaults,
                                      .message = sqlite3_errmsg(db)});
  }

  for (const auto &formula : formulas) {
    if (sqlite3_bind_text(stmt, 1, formula.name.c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 2, formula.expr.c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK) {
      sqlite3_finalize(stmt_defaults);
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      sqlite3_finalize(stmt_defaults);
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);

    for (const auto &[var, value] : formula.defaults) {
      if (sqlite3_bind_text(stmt_defaults, 1, formula.name.c_str(), -1,
                            SQLITE_TRANSIENT) != SQLITE_OK ||
          sqlite3_bind_text(stmt_defaults, 2, var.c_str(), -1,
                            SQLITE_TRANSIENT) != SQLITE_OK ||
          sqlite3_bind_double(stmt_defaults, 3, value) != SQLITE_OK) {
        sqlite3_finalize(stmt_defaults);
        sqlite3_finalize(stmt);
        rollback();
        return ps_error(ps_db_query_err_t{.query = sql_defaults,
                                          .message = sqlite3_errmsg(db)});
      }
      if (sqlite3_step(stmt_defaults) != SQLITE_DONE) {
        sqlite3_finalize(stmt_defaults);
        sqlite3_finalize(stmt);
        rollback();
        return ps_error(ps_db_query_err_t{.query = sql_defaults,
                                          .message = sqlite3_errmsg(db)});
      }
      sqlite3_clear_bindings(stmt_defaults);
      sqlite3_reset(stmt_defaults);
    }
  }

  sqlite3_finalize(stmt_defaults);
  sqlite3_finalize(stmt);

  if (sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = "COMMIT", .message = sqlite3_errmsg(db)});
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

  auto rollback = [&] {
    sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
  };

  if (sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = "BEGIN", .message = sqlite3_errmsg(db)});
  }

  // Add to formulas table. A plain insert fails if the formula already exists.
  auto sql = "INSERT INTO formulas (name, expression) VALUES (?, ?);";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }

  auto sql_defaults = "INSERT OR REPLACE INTO formula_defaults (formula, var, "
                      "value) VALUES (?, ?, ?);";
  sqlite3_stmt *stmt_defaults = nullptr;
  if (sqlite3_prepare_v2(db, sql_defaults, -1, &stmt_defaults, nullptr) !=
      SQLITE_OK) {
    sqlite3_finalize(stmt);
    rollback();
    return ps_error(ps_db_query_err_t{.query = sql_defaults,
                                      .message = sqlite3_errmsg(db)});
  }

  for (const auto &formula : formulas) {
    if (sqlite3_bind_text(stmt, 1, formula.name.c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 2, formula.expr.c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK) {
      sqlite3_finalize(stmt_defaults);
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      sqlite3_finalize(stmt_defaults);
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

    for (const auto &[var, value] : formula.defaults) {
      if (sqlite3_bind_text(stmt_defaults, 1, formula.name.c_str(), -1,
                            SQLITE_TRANSIENT) != SQLITE_OK ||
          sqlite3_bind_text(stmt_defaults, 2, var.c_str(), -1,
                            SQLITE_TRANSIENT) != SQLITE_OK ||
          sqlite3_bind_double(stmt_defaults, 3, value) != SQLITE_OK) {
        sqlite3_finalize(stmt_defaults);
        sqlite3_finalize(stmt);
        rollback();
        return ps_error(ps_db_query_err_t{.query = sql_defaults,
                                          .message = sqlite3_errmsg(db)});
      }
      if (sqlite3_step(stmt_defaults) != SQLITE_DONE) {
        sqlite3_finalize(stmt_defaults);
        sqlite3_finalize(stmt);
        rollback();
        return ps_error(ps_db_query_err_t{.query = sql_defaults,
                                          .message = sqlite3_errmsg(db)});
      }
      sqlite3_clear_bindings(stmt_defaults);
      sqlite3_reset(stmt_defaults);
    }
  }

  sqlite3_finalize(stmt_defaults);
  sqlite3_finalize(stmt);

  if (sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = "COMMIT", .message = sqlite3_errmsg(db)});
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

  auto rollback = [&] {
    sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
  };

  if (sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = "BEGIN", .message = sqlite3_errmsg(db)});
  }

  // Update the formulas row. If no row is affected the formula is absent.
  auto sql = "UPDATE formulas SET expression = ? WHERE name = ?;";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }

  auto sql_defaults = "INSERT OR REPLACE INTO formula_defaults (formula, var, "
                      "value) VALUES (?, ?, ?);";
  sqlite3_stmt *stmt_defaults = nullptr;
  if (sqlite3_prepare_v2(db, sql_defaults, -1, &stmt_defaults, nullptr) !=
      SQLITE_OK) {
    sqlite3_finalize(stmt);
    rollback();
    return ps_error(ps_db_query_err_t{.query = sql_defaults,
                                      .message = sqlite3_errmsg(db)});
  }

  for (const auto &formula : formulas) {
    if (sqlite3_bind_text(stmt, 1, formula.expr.c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 2, formula.name.c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK) {
      sqlite3_finalize(stmt_defaults);
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      sqlite3_finalize(stmt_defaults);
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
    if (sqlite3_changes(db) == 0) {
      sqlite3_finalize(stmt_defaults);
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(ps_db_not_found_t{.key = "formula." + formula.name});
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);

    for (const auto &[var, value] : formula.defaults) {
      if (sqlite3_bind_text(stmt_defaults, 1, formula.name.c_str(), -1,
                            SQLITE_TRANSIENT) != SQLITE_OK ||
          sqlite3_bind_text(stmt_defaults, 2, var.c_str(), -1,
                            SQLITE_TRANSIENT) != SQLITE_OK ||
          sqlite3_bind_double(stmt_defaults, 3, value) != SQLITE_OK) {
        sqlite3_finalize(stmt_defaults);
        sqlite3_finalize(stmt);
        rollback();
        return ps_error(ps_db_query_err_t{.query = sql_defaults,
                                          .message = sqlite3_errmsg(db)});
      }
      if (sqlite3_step(stmt_defaults) != SQLITE_DONE) {
        sqlite3_finalize(stmt_defaults);
        sqlite3_finalize(stmt);
        rollback();
        return ps_error(ps_db_query_err_t{.query = sql_defaults,
                                          .message = sqlite3_errmsg(db)});
      }
      sqlite3_clear_bindings(stmt_defaults);
      sqlite3_reset(stmt_defaults);
    }
  }

  sqlite3_finalize(stmt_defaults);
  sqlite3_finalize(stmt);

  if (sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = "COMMIT", .message = sqlite3_errmsg(db)});
  }

  return ps_success_t{};
}

ps_result_t<ps_success_t>
ps_db_sqlite_formula_update(sqlite3 *db, const ps_formula_t &formula) {
  return ps_db_sqlite_formulas_update(db, {formula});
}
