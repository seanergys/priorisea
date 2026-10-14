// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <sqlite3.h>

using ps_db_formulas_get_t = std::map<std::string, ps_result_t<ps_formula_t>>;

/**
 * Resets the formulas in the database to their default values.
 *
 * The reset is all-or-none: every requested name must be a valid default
 * formula name, otherwise nothing is reset.
 *
 * @param names The names of the formulas to reset.
 * @return A result indicating the success of the operation.
 */
ps_result_t<ps_success_t>
ps_db_formulas_reset(sqlite3 *db, const std::vector<std::string> &names);

/**
 * Initializes the formulas in the database.
 *
 * @return A result indicating the success of the operation.
 */
ps_result_t<ps_success_t> ps_db_formulas_init(sqlite3 *db);
