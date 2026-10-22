// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <CLI/CLI.hpp>

/**
 * Default path for the database file
 */
inline const std::string DATABASE_PATH_DEFAULT =
    "/var/lib/priorisea/priorisea.db";

/**
 * Register the database commands with the CLI application
 *
 * @param app The CLI application to register the commands with
 */
void register_database_commands(CLI::App *app);
