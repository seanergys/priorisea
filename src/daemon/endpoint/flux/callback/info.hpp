#pragma once
#include "src/daemon/endpoint/flux/endpoint.hpp"

/**
 * Handles the priorisea.daemon.info.get RPC
 *
 * Responds with the database schema version and the info row, or an error if
 * they cannot be read
 *
 * @param args The Flux RPC callback args
 * @return 0 on success, or -1 if the response could not be sent
 */
int ps_daemon_endpoint_flux_info_get(
    const ps_daemon_endpoint_flux_callback_args_t &args);

/**
 * Handles the priorisea.daemon.info.user_period.update RPC
 *
 * Updates the user_period in the info row
 *
 * @param args The Flux RPC callback args
 * @return 0 on success, or -1 if the response could not be sent
 */
int ps_daemon_endpoint_flux_info_user_period_update(
    const ps_daemon_endpoint_flux_callback_args_t &args);
