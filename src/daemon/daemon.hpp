#pragma once
#include "src/lib/types.hpp"

/**
 * Starts the daemon with the specified database path
 *
 * @param config The daemon config
 */
void ps_daemon_run(const ps_daemon_config_t &config);
