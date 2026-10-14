// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/database/formula.hpp"
#include "src/lib/types.hpp"
#include <sqlite3.h>

ps_result_t<ps_formula_t> ps_db_sqlite_formula_get(sqlite3 *db,
                                                   const std::string &name);

ps_result_t<ps_db_formulas_get_t>
ps_db_sqlite_formulas_get(sqlite3 *db, const std::vector<std::string> &names);

ps_result_t<std::vector<ps_formula_t>> ps_db_sqlite_formulas_list(sqlite3 *db);

ps_result_t<ps_success_t>
ps_db_sqlite_formulas_set(sqlite3 *db,
                          const std::vector<ps_formula_t> &formulas);

ps_result_t<ps_success_t> ps_db_sqlite_formula_set(sqlite3 *db,
                                                   const ps_formula_t &formula);

ps_result_t<ps_success_t>
ps_db_sqlite_formulas_insert(sqlite3 *db,
                             const std::vector<ps_formula_t> &formulas);

ps_result_t<ps_success_t>
ps_db_sqlite_formula_insert(sqlite3 *db, const ps_formula_t &formula);

ps_result_t<ps_success_t>
ps_db_sqlite_formulas_update(sqlite3 *db,
                             const std::vector<ps_formula_t> &formulas);

ps_result_t<ps_success_t>
ps_db_sqlite_formula_update(sqlite3 *db, const ps_formula_t &formula);
