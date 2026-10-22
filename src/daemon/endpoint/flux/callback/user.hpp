// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/daemon/endpoint/flux/endpoint.hpp"

/**
 * Handles the priorisea.daemon.user.get RPC
 *
 * Fetches the requested user from the database, enforcing that callers may only
 * read their own record, and responds with it or an error
 *
 * @param args The Flux RPC callback args
 * @return 0 on success, or -1 if the response could not be sent
 */
int ps_daemon_endpoint_flux_user_get(
    const ps_daemon_endpoint_flux_callback_args_t &args);
/**
 * Handles the priorisea.daemon.user.list RPC
 *
 * Responds with the requested users as an array (all of them when no ids are
 * given), or an error if the request is malformed
 *
 * @param args The Flux RPC callback args
 * @return 0 on success, or -1 if the response could not be sent
 */
int ps_daemon_endpoint_flux_user_list(
    const ps_daemon_endpoint_flux_callback_args_t &args);
/**
 * Handles the priorisea.daemon.user.update RPC
 *
 * Recomputes user scores from their jobs using the job_weight formula and
 * stores them, for all users or only the requested ones
 *
 * @param args The Flux RPC callback args
 * @return 0 on success, or -1 if the response could not be sent
 */
int ps_daemon_endpoint_flux_user_update(
    const ps_daemon_endpoint_flux_callback_args_t &args);
