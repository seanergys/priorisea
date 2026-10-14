// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include <sqlite3.h>

ps_result_t<double> ps_db_sqlite_t_inactive_get(sqlite3 *db);

ps_result_t<ps_success_t> ps_db_sqlite_t_inactive_set(sqlite3 *db,
                                                      double t_inactive);
