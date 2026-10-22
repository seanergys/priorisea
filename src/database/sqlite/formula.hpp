// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/database/formula.hpp"
#include "src/lib/types.hpp"
#include <sqlite3.h>

/**
 * Fetches a single formula by name
 *
 * @param db The SQLite database
 * @param name The name of the formula to fetch
 * @return The formula, or an error if it does not exist
 */
ps_result_t<ps_formula_t> ps_db_sqlite_formula_get(sqlite3 *db,
                                                   const std::string &name);

/**
 * Fetches multiple formulas by name
 *
 * Returns one entry per requested name, with an error for any missing name
 *
 * @param db The SQLite database
 * @param names The names of the formulas to fetch
 * @return A map of name to result
 */
ps_result_t<ps_db_formulas_get_t>
ps_db_sqlite_formulas_get(sqlite3 *db, const std::vector<std::string> &names);

/**
 * Lists every formula stored in the database
 *
 * @param db The SQLite database
 * @return The formulas
 */
ps_result_t<std::vector<ps_formula_t>> ps_db_sqlite_formulas_list(sqlite3 *db);

/**
 * Inserts or replaces the formulas in a single transaction
 *
 * @param db The SQLite database
 * @param formulas The formulas to store
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t>
ps_db_sqlite_formulas_set(sqlite3 *db,
                          const std::vector<ps_formula_t> &formulas);

/**
 * Inserts or replaces a single formula in a single transaction
 *
 * @param db The SQLite database
 * @param formula The formula to store
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t> ps_db_sqlite_formula_set(sqlite3 *db,
                                                   const ps_formula_t &formula);

/**
 * Inserts new formulas, failing if any of them already exists
 *
 * @param db The SQLite database
 * @param formulas The formulas to insert
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t>
ps_db_sqlite_formulas_insert(sqlite3 *db,
                             const std::vector<ps_formula_t> &formulas);

/**
 * Inserts a new formula, failing if it already exists
 *
 * @param db The SQLite database
 * @param formula The formula to insert
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t>
ps_db_sqlite_formula_insert(sqlite3 *db, const ps_formula_t &formula);

/**
 * Updates existing formulas, failing if any of them is missing
 *
 * @param db The SQLite database
 * @param formulas The formulas to update
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t>
ps_db_sqlite_formulas_update(sqlite3 *db,
                             const std::vector<ps_formula_t> &formulas);

/**
 * Updates an existing formula, failing if it is missing
 *
 * @param db The SQLite database
 * @param formula The formula to update
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t>
ps_db_sqlite_formula_update(sqlite3 *db, const ps_formula_t &formula);
