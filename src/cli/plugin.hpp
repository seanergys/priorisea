// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <CLI/CLI.hpp>

/**
 * Registers the plugin commands for the CLI application.
 *
 * @param app The CLI application to register the commands with.
 */
void register_plugin_commands(CLI::App *app);
