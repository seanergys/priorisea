// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/database/sqlite/decay.hpp"

static const ps_decay_t DEFAULT_DECAY = {
    {.duration = 86400, .weight = 1},
    {.duration = 604800, .weight = 0.5},
    {.duration = 1209600, .weight = 0.25},
};

ps_result_t<ps_success_t> ps_db_decay_reset(sqlite3 *db) {
  return ps_db_sqlite_decay_set(db, DEFAULT_DECAY);
}

ps_result_t<ps_success_t> ps_db_decay_init(sqlite3 *db) {
  auto get_res = ps_db_sqlite_decay_get(db);
  if (!ps_is_ok(get_res)) {
    return ps_db_decay_reset(db);
  }
  return ps_success_t{};
}
