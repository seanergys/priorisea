// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include <sqlite3.h>
#include <string>

inline constexpr int SQLITE_DB_VERSION = 1;

/**
 * Executes an SQL statement on the given SQLite database.
 *
 * @param db The SQLite database to execute the statement on.
 * @param sql The SQL statement to execute.
 * @return A success if the statement was executed successfully.
 */
ps_result_t<ps_success_t> ps_db_sqlite_execute(sqlite3 *db,
                                               const std::string &sql);

/**
 * Creates a SQLite database at the specified path.
 *
 * @param path The path to the database file.
 * @return A success if the database was created successfully.
 */
ps_result_t<sqlite3 *> ps_db_sqlite_create(const std::string &path);

/**
 * Opens a SQLite database at the specified path.
 *
 * @param path The path to the database file.
 * @return A pointer to the SQLite database if successful.
 */
ps_result_t<sqlite3 *> ps_db_sqlite_open(const std::string &path);

/**
 * Gets the user version of the SQLite database at the specified path.
 *
 * @param db The SQLite database to get the user version from.
 * @return The user version of the database.
 */
ps_result_t<int> ps_db_sqlite_get_user_version(sqlite3 *db);

/**
 * Sets the user version of the SQLite database at the specified path.
 *
 * @param db The SQLite database to set the user version on.
 * @param version The user version to set.
 * @return A success if the user version was set successfully.
 */
ps_result_t<ps_success_t> ps_db_sqlite_set_user_version(sqlite3 *db,
                                                        int version);

/**
 * Verifies that the SQLite database is valid.
 *
 * @param db The SQLite database to verify.
 * @return A success if the database is valid.
 */
ps_result_t<ps_success_t> ps_db_sqlite_verify(sqlite3 *db);

bool ps_db_sqlite_is_constraint(sqlite3 *db);
