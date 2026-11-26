#pragma once
#include <CLI/CLI.hpp>

/**
 * Default endpoint for the SEANERBUS daemon
 */
inline const std::string SEANERBUS_ENDPOINT_DEFAULT =
    "seanerbroker://localhost:5398";

/**
 * Register the daemon commands with the CLI application
 *
 * @param app The CLI application to register the commands with
 */
void register_daemon_commands(CLI::App *app);
