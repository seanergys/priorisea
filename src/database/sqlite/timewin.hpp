// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/database/timewin.hpp"
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <sqlite3.h>

ps_result_t<ps_timewin_t> ps_db_sqlite_timewin_get(sqlite3 *db,
                                                   const std::string &name);

ps_result_t<ps_db_timewins_get_t>
ps_db_sqlite_timewins_get(sqlite3 *db, const std::vector<std::string> &names);

ps_result_t<std::vector<ps_timewin_t>> ps_db_sqlite_timewins_list(sqlite3 *db);

ps_result_t<ps_success_t>
ps_db_sqlite_timewins_set(sqlite3 *db,
                          const std::vector<ps_timewin_t> &timewins);

ps_result_t<ps_success_t> ps_db_sqlite_timewin_set(sqlite3 *db,
                                                   const ps_timewin_t &timewin);

ps_result_t<ps_success_t>
ps_db_sqlite_timewins_insert(sqlite3 *db,
                             const std::vector<ps_timewin_t> &timewins);

ps_result_t<ps_success_t>
ps_db_sqlite_timewin_insert(sqlite3 *db, const ps_timewin_t &timewin);

ps_result_t<ps_success_t>
ps_db_sqlite_timewins_update(sqlite3 *db,
                             const std::vector<ps_timewin_t> &timewins);

ps_result_t<ps_success_t>
ps_db_sqlite_timewin_update(sqlite3 *db, const ps_timewin_t &timewin);

ps_result_t<ps_success_t> ps_db_sqlite_timewin_del(sqlite3 *db,
                                                   const std::string &name);

ps_result_t<ps_success_t>
ps_db_sqlite_timewins_del(sqlite3 *db, const std::vector<std::string> &names);
