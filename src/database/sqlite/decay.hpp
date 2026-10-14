// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include <sqlite3.h>

ps_result_t<ps_decay_t> ps_db_sqlite_decay_get(sqlite3 *db);

ps_result_t<ps_success_t> ps_db_sqlite_decay_set(sqlite3 *db,
                                                 const ps_decay_t &decay);

ps_result_t<ps_success_t> ps_db_sqlite_decay_insert(sqlite3 *db,
                                                    const ps_decay_t &decay);

ps_result_t<ps_success_t> ps_db_sqlite_decay_update(sqlite3 *db,
                                                    const ps_decay_t &decay);
