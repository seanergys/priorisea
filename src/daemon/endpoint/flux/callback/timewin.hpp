#pragma once
#include "src/daemon/endpoint/flux/endpoint.hpp"

/**
 * Handles the priorisea.daemon.timewin.get RPC
 *
 * Fetches the requested time window from the database and responds with it, or
 * an error if the request is malformed or the time window is missing
 *
 * @param args The Flux RPC callback args
 * @return 0 on success, or -1 if the response could not be sent
 */
int ps_daemon_endpoint_flux_timewin_get(
    const ps_daemon_endpoint_flux_callback_args_t &args);
/**
 * Handles the priorisea.daemon.timewin.list RPC
 *
 * Responds with the requested time windows as an array (all of them when no
 * names are given), or an error if the request is malformed
 *
 * @param args The Flux RPC callback args
 * @return 0 on success, or -1 if the response could not be sent
 */
int ps_daemon_endpoint_flux_timewin_list(
    const ps_daemon_endpoint_flux_callback_args_t &args);
/**
 * Handles the priorisea.daemon.timewin.update RPC
 *
 * Validates and updates a time window in the database, or responds with an
 * error if it is invalid or does not exist
 *
 * @param args The Flux RPC callback args
 * @return 0 on success, or -1 if the response could not be sent
 */
int ps_daemon_endpoint_flux_timewin_update(
    const ps_daemon_endpoint_flux_callback_args_t &args);
/**
 * Handles the priorisea.daemon.timewin.create RPC
 *
 * Validates and creates a new time window in the database, or responds with an
 * error if it is invalid or already exists
 *
 * @param args The Flux RPC callback args
 * @return 0 on success, or -1 if the response could not be sent
 */
int ps_daemon_endpoint_flux_timewin_create(
    const ps_daemon_endpoint_flux_callback_args_t &args);
/**
 * Handles the priorisea.daemon.timewin.delete RPC
 *
 * Deletes the given time windows from the database, or responds with an error
 * if the request is malformed
 *
 * @param args The Flux RPC callback args
 * @return 0 on success, or -1 if the response could not be sent
 */
int ps_daemon_endpoint_flux_timewin_delete(
    const ps_daemon_endpoint_flux_callback_args_t &args);
