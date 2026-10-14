// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <CLI/CLI.hpp>

inline const std::string DATABASE_PATH_DEFAULT =
    "/var/lib/priorisea/priorisea.db";

void register_database_commands(CLI::App *app);
