// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include <sqlite3.h>

/**
 * Resets the info in the database
 *
 * @param user_period The default user_period
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t> ps_db_info_reset(sqlite3 *db, double user_period);

/**
 * Initializes the info in the database
 *
 * @param user_period The default user_period
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t> ps_db_info_init(sqlite3 *db, double user_period);
