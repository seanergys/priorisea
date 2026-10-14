// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/database/info.hpp"
#include "src/database/sqlite/info.hpp"
#include "src/lib/error.hpp"

ps_result_t<ps_success_t> ps_db_info_reset(sqlite3 *db) {
  return ps_db_sqlite_t_inactive_set(db, 0);
}

ps_result_t<ps_success_t> ps_db_info_init(sqlite3 *db) {
  auto get_res = ps_db_sqlite_t_inactive_get(db);
  if (!ps_is_ok(get_res)) {
    return ps_db_info_reset(db);
  }
  return ps_success_t{};
}
