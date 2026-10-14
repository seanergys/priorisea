// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <CLI/CLI.hpp>

inline const std::string SEANERBUS_ENDPOINT_DEFAULT =
    "demobus://localhost:5398";

void register_daemon_commands(CLI::App *app);
