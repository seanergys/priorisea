// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <CLI/CLI.hpp>
#include <lib/types.hpp>
#include <string>
void register_score_commands(CLI::App *app, const std::string &config_path);
