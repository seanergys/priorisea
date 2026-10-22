// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/daemon/endpoint/flux/endpoint.hpp"

/**
 * Handles the priorisea.daemon.plugin.sync RPC
 *
 * Forwards the job_priority formula and the current user scores to the
 * job-manager plugin's sync RPC, or responds with an error
 *
 * @param args The Flux RPC callback args
 * @return 0 on success, or -1 if the response could not be sent
 */
int ps_daemon_endpoint_flux_plugin_sync(
    const ps_daemon_endpoint_flux_callback_args_t &args);
/**
 * Handles the priorisea.daemon.plugin.debug RPC
 *
 * Forwards a debug request to the job-manager plugin and relays its response
 * back to the caller
 *
 * @param args The Flux RPC callback args
 * @return 0 on success, or -1 if the response could not be sent
 */
int ps_daemon_endpoint_flux_plugin_debug(
    const ps_daemon_endpoint_flux_callback_args_t &args);
