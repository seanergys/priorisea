// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/database/database.hpp"
#include "src/database/formula.hpp"
#include "src/database/info.hpp"
#include "src/database/sqlite/sqlite.hpp"
#include "src/lib/error.hpp"
#include <unistd.h>

ps_result_t<ps_success_t> ps_db_database_create(const std::string &path,
                                                double user_period) {
  auto db_res = ps_db_sqlite_create(path);
  if (!ps_is_ok(db_res)) {
    return ps_forward_error<ps_success_t>(db_res);
  }
  auto db = ps_unwrap(db_res);

  auto abort = [&] {
    sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
    sqlite3_close(db);
    unlink(path.c_str());
  };

  // Enable foreign-key enforcement
  auto fk_res = ps_db_sqlite_execute(db, "PRAGMA foreign_keys = ON;");
  if (!ps_is_ok(fk_res)) {
    abort();
    return fk_res;
  }

  if (sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr) != SQLITE_OK) {
    sqlite3_close(db);
    return ps_error(
        ps_db_query_err_t{.query = "BEGIN", .message = sqlite3_errmsg(db)});
  }

  // Create the schema
  auto schema_res = ps_db_sqlite_create_schema(db);
  if (!ps_is_ok(schema_res)) {
    abort();
    return schema_res;
  }

  // Seed the default formulas
  auto formulas_res = ps_db_formulas_init(db);
  if (!ps_is_ok(formulas_res)) {
    abort();
    return formulas_res;
  }

  // Seed the info row
  auto info_res = ps_db_info_init(db, user_period);
  if (!ps_is_ok(info_res)) {
    abort();
    return info_res;
  }

  if (sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr) != SQLITE_OK) {
    abort();
    return ps_error(
        ps_db_query_err_t{.query = "COMMIT", .message = sqlite3_errmsg(db)});
  }

  sqlite3_close(db);
  return ps_success_t{};
}
