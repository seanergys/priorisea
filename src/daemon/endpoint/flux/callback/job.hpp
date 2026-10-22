// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/daemon/endpoint/flux/endpoint.hpp"

/**
 * Handles the priorisea.daemon.job.get RPC
 *
 * Fetches the requested job from the database, enforcing that callers may only
 * read their own jobs, and responds with it or an error
 *
 * @param args The Flux RPC callback args
 * @return 0 on success, or -1 if the response could not be sent
 */
int ps_daemon_endpoint_flux_job_get(
    const ps_daemon_endpoint_flux_callback_args_t &args);
/**
 * Handles the priorisea.daemon.job.list RPC
 *
 * Responds with the authorized jobs as an array (all of them when no ids are
 * given, capped by the requested limit), or an error if the request is
 * malformed
 *
 * @param args The Flux RPC callback args
 * @return 0 on success, or -1 if the response could not be sent
 */
int ps_daemon_endpoint_flux_job_list(
    const ps_daemon_endpoint_flux_callback_args_t &args);
/**
 * Handles the priorisea.daemon.job.update RPC
 *
 * Scores the given or newly inactive jobs with the job_score formula and stores
 * them, advancing the t_inactive cursor in the same transaction when scanning
 * new jobs
 *
 * @param args The Flux RPC callback args
 * @return 0 on success, or -1 if the response could not be sent
 */
int ps_daemon_endpoint_flux_job_update(
    const ps_daemon_endpoint_flux_callback_args_t &args);
