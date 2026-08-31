// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "lib/types.hpp"
#include <CLI/CLI.hpp>
#include <string>
void register_config_commands(CLI::App *app, const std::string &config_path);
