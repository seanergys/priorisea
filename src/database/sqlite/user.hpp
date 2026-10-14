// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/database/user.hpp"
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <flux/core.h>
#include <sqlite3.h>
#include <vector>

ps_result_t<ps_user_t> ps_db_sqlite_user_get(sqlite3 *db,
                                             const std::string &username);

ps_result_t<ps_db_sqlite_users_get_t>
ps_db_sqlite_users_get(sqlite3 *db, const std::vector<std::string> &usernames);

ps_result_t<std::vector<ps_user_t>> ps_db_sqlite_users_list(sqlite3 *db);

ps_result_t<ps_success_t>
ps_db_sqlite_users_insert(sqlite3 *db, const std::vector<ps_user_t> &users);

ps_result_t<ps_success_t> ps_db_sqlite_user_insert(sqlite3 *db,
                                                   const ps_user_t &user);

ps_result_t<ps_success_t>
ps_db_sqlite_users_update(sqlite3 *db, const std::vector<ps_user_t> &users);

ps_result_t<ps_success_t> ps_db_sqlite_user_update(sqlite3 *db,
                                                   const ps_user_t &user);

ps_result_t<ps_success_t>
ps_db_sqlite_users_set(sqlite3 *db, const std::vector<ps_user_t> &users);

ps_result_t<ps_success_t> ps_db_sqlite_user_set(sqlite3 *db,
                                                const ps_user_t &user);
