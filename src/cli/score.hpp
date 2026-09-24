// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <CLI/CLI.hpp>
#include <lib/types.hpp>
#include <string>

/**
 * Registers the score commands for the CLI application.
 *
 * @param app The CLI application to register the commands with.
 * @param config_path The path to the configuration file.
 */
void register_score_commands(CLI::App *app, const std::string &config_path);
