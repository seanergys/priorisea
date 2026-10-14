// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include <sqlite3.h>

/**
 * Resets the decay in the database.
 *
 * @return A result indicating success or failure.
 */
ps_result_t<ps_success_t> ps_db_decay_reset(sqlite3 *db);

/**
 * Initializes the decay in the database.
 *
 * @return A result indicating success or failure.
 */
ps_result_t<ps_success_t> ps_db_decay_init(sqlite3 *db);
