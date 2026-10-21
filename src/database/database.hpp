// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include <sqlite3.h>
#include <string>

/**
 * Atomically creates the database: the schema, the default formulas, and the
 * info row are written in a single transaction.
 *
 * @param path The path of the database to create.
 * @param user_period The default user_period.
 * @return The opened database, or an error.
 */
ps_result_t<sqlite3 *> ps_db_database_create(const std::string &path,
                                             double user_period);
