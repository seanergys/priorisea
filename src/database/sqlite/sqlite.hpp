#pragma once
#include "src/lib/error.hpp"
#include <sqlite3.h>
#include <string>

/**
 * Expected SQLite user_version of the schema
 */
inline constexpr int SQLITE_DB_VERSION = 2;

/**
 * Executes an SQL statement on the given SQLite database
 *
 * @param db The SQLite database to execute the statement on
 * @param sql The SQL statement to execute
 * @return A success if the statement was executed successfully
 */
ps_result_t<ps_success_t> ps_db_sqlite_execute(sqlite3 *db,
                                               const std::string &sql);

/**
 * Creates a new database file at the specified path (fails if it already
 * exists) and opens it. The schema is not created here; call
 * ps_db_sqlite_create_schema inside a transaction to initialize it
 *
 * @param path The path to the database file
 * @return The opened database if successful
 */
ps_result_t<sqlite3 *> ps_db_sqlite_create(const std::string &path);

/**
 * Creates the database schema (user_version and tables) on an open database
 * Designed to run inside a transaction so it can be committed atomically with
 * the seed data
 *
 * @param db The open SQLite database
 * @return A success if the schema was created successfully
 */
ps_result_t<ps_success_t> ps_db_sqlite_create_schema(sqlite3 *db);

/**
 * Opens a SQLite database at the specified path
 *
 * @param path The path to the database file
 * @return A pointer to the SQLite database if successful
 */
ps_result_t<sqlite3 *> ps_db_sqlite_open(const std::string &path);

/**
 * Gets the user version of the SQLite database at the specified path
 *
 * @param db The SQLite database to get the user version from
 * @return The user version of the database
 */
ps_result_t<int> ps_db_sqlite_get_user_version(sqlite3 *db);

/**
 * Sets the user version of the SQLite database at the specified path
 *
 * @param db The SQLite database to set the user version on
 * @param version The user version to set
 * @return A success if the user version was set successfully
 */
ps_result_t<ps_success_t> ps_db_sqlite_set_user_version(sqlite3 *db,
                                                        int version);

/**
 * Verifies that the SQLite database is valid
 *
 * @param db The SQLite database to verify
 * @return A success if the database is valid
 */
ps_result_t<ps_success_t> ps_db_sqlite_verify(sqlite3 *db);

/**
 * Checks whether the last SQLite error was a constraint violation
 */
bool ps_db_sqlite_is_constraint(sqlite3 *db);
