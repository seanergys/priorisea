#pragma once
#include "src/database/user.hpp"
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include "src/lib/util.hpp"
#include <flux/core.h>
#include <sqlite3.h>
#include <vector>

/**
 * Fetches a single user by id
 *
 * @param db The SQLite database
 * @param userid The id of the user to fetch
 * @return The user, or an error if it does not exist
 */
ps_result_t<ps_user_t> ps_db_sqlite_user_get(sqlite3 *db, uid_t userid);

/**
 * Fetches multiple users by id
 *
 * Returns one entry per requested id, with an error for any missing id
 *
 * @param db The SQLite database
 * @param userids The ids of the users to fetch
 * @return A map of id to result
 */
ps_result_t<ps_db_sqlite_users_get_t>
ps_db_sqlite_users_get(sqlite3 *db, const std::vector<uid_t> &userids);

/**
 * Lists every user stored in the database
 *
 * @param db The SQLite database
 * @param order The order of the listing (defaults to descending)
 * @return The users
 */
ps_result_t<std::vector<ps_user_t>>
ps_db_sqlite_users_list(sqlite3 *db, ps_db_order_t order = ps_db_order_t::desc);

/**
 * Inserts new users, failing if any of them already exists
 *
 * @param db The SQLite database
 * @param users The users to insert
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t>
ps_db_sqlite_users_insert(sqlite3 *db, const std::vector<ps_user_t> &users);

/**
 * Inserts a new user, failing if it already exists
 *
 * @param db The SQLite database
 * @param user The user to insert
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t> ps_db_sqlite_user_insert(sqlite3 *db,
                                                   const ps_user_t &user);

/**
 * Updates existing users, failing if any of them is missing
 *
 * @param db The SQLite database
 * @param users The users to update
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t>
ps_db_sqlite_users_update(sqlite3 *db, const std::vector<ps_user_t> &users);

/**
 * Updates an existing user, failing if it is missing
 *
 * @param db The SQLite database
 * @param user The user to update
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t> ps_db_sqlite_user_update(sqlite3 *db,
                                                   const ps_user_t &user);

/**
 * Inserts or replaces the users in a single transaction
 *
 * @param db The SQLite database
 * @param users The users to store
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t>
ps_db_sqlite_users_set(sqlite3 *db, const std::vector<ps_user_t> &users);

/**
 * Inserts or replaces a single user in a single transaction
 *
 * @param db The SQLite database
 * @param user The user to store
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t> ps_db_sqlite_user_set(sqlite3 *db,
                                                const ps_user_t &user);
