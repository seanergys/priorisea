#pragma once
#include <CLI/CLI.hpp>

/**
 * Register the plugin commands with the CLI application
 *
 * @param app The CLI application to register the commands with
 */
void register_plugin_commands(CLI::App *app);
