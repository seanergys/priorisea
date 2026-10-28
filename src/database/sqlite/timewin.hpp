#pragma once
#include "src/database/timewin.hpp"
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <sqlite3.h>

/**
 * Fetches a single time window by name
 *
 * @param db The SQLite database
 * @param name The name of the time window to fetch
 * @return The time window, or an error if it does not exist
 */
ps_result_t<ps_timewin_t> ps_db_sqlite_timewin_get(sqlite3 *db,
                                                   const std::string &name);

/**
 * Fetches multiple time windows by name
 *
 * Returns one entry per requested name, with an error for any missing name
 *
 * @param db The SQLite database
 * @param names The names of the time windows to fetch
 * @return A map of name to result
 */
ps_result_t<ps_db_timewins_get_t>
ps_db_sqlite_timewins_get(sqlite3 *db, const std::vector<std::string> &names);

/**
 * Lists every time window stored in the database
 *
 * @param db The SQLite database
 * @return The time windows
 */
ps_result_t<std::vector<ps_timewin_t>> ps_db_sqlite_timewins_list(sqlite3 *db);

/**
 * Inserts or replaces the time windows in a single transaction
 *
 * @param db The SQLite database
 * @param timewins The time windows to store
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t>
ps_db_sqlite_timewins_set(sqlite3 *db,
                          const std::vector<ps_timewin_t> &timewins);

/**
 * Inserts or replaces a single time window in a single transaction
 *
 * @param db The SQLite database
 * @param timewin The time window to store
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t> ps_db_sqlite_timewin_set(sqlite3 *db,
                                                   const ps_timewin_t &timewin);

/**
 * Inserts new time windows, failing if any of them already exists
 *
 * @param db The SQLite database
 * @param timewins The time windows to insert
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t>
ps_db_sqlite_timewins_insert(sqlite3 *db,
                             const std::vector<ps_timewin_t> &timewins);

/**
 * Inserts a new time window, failing if it already exists
 *
 * @param db The SQLite database
 * @param timewin The time window to insert
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t>
ps_db_sqlite_timewin_insert(sqlite3 *db, const ps_timewin_t &timewin);

/**
 * Updates existing time windows, failing if any of them is missing
 *
 * @param db The SQLite database
 * @param timewins The time windows to update
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t>
ps_db_sqlite_timewins_update(sqlite3 *db,
                             const std::vector<ps_timewin_t> &timewins);

/**
 * Updates an existing time window, failing if it is missing
 *
 * @param db The SQLite database
 * @param timewin The time window to update
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t>
ps_db_sqlite_timewin_update(sqlite3 *db, const ps_timewin_t &timewin);

/**
 * Deletes the time window with the given name
 *
 * @param db The SQLite database
 * @param name The name of the time window to delete
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t> ps_db_sqlite_timewin_del(sqlite3 *db,
                                                   const std::string &name);

/**
 * Deletes the time windows with the given names
 *
 * @param db The SQLite database
 * @param names The names of the time windows to delete
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t>
ps_db_sqlite_timewins_del(sqlite3 *db, const std::vector<std::string> &names);
