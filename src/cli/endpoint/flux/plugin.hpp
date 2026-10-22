// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/cli/endpoint/plugin.hpp"
#include "src/lib/error.hpp"

/**
 * Trigger a plugin data sync on the daemon
 */
ps_result_t<ps_success_t> ps_cli_endpoint_flux_plugin_sync();

/**
 * Fetch plugin debug data from the daemon
 * including the formula and per-user scores
 */
ps_result_t<ps_cli_endpoint_plugin_debug_t> ps_cli_endpoint_flux_plugin_debug();
