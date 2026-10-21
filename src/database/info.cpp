// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/database/sqlite/info.hpp"
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"

ps_result_t<ps_success_t> ps_db_info_reset(sqlite3 *db, double user_period) {
  return ps_db_sqlite_info_set(
      db, ps_db_info_t{.t_inactive = 0, .user_period = user_period});
}

ps_result_t<ps_success_t> ps_db_info_init(sqlite3 *db, double user_period) {
  // If the info row already exists, leave it untouched; otherwise create it.
  auto info_res = ps_db_sqlite_info_get(db);
  if (!ps_is_ok(info_res)) {
    return ps_db_info_reset(db, user_period);
  }
  return ps_success_t{};
}
