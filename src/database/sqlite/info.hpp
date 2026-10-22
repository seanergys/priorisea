#pragma once
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <sqlite3.h>

/**
 * Reads the whole database_info row (the t_inactive cursor and user_period) in
 * a single query
 *
 * @param db The SQLite database
 * @return The info row, or an error if the row does not exist
 */
ps_result_t<ps_db_info_t> ps_db_sqlite_info_get(sqlite3 *db);

/**
 * Writes the whole database_info row (the t_inactive cursor and user_period) in
 * a single atomic statement, so neither column can be dropped
 *
 * @param db The SQLite database
 * @param info The info row to write
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t> ps_db_sqlite_info_set(sqlite3 *db,
                                                const ps_db_info_t &info);
