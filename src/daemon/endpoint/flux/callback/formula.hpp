#pragma once
#include "src/daemon/endpoint/flux/endpoint.hpp"

/**
 * Handles the priorisea.daemon.formula.get RPC
 *
 * Fetches the requested formula from the database and responds with it, or an
 * error if the request is malformed or the formula is missing
 *
 * @param args The Flux RPC callback args
 * @return 0 on success, or -1 if the response could not be sent
 */
int ps_daemon_endpoint_flux_formula_get(
    const ps_daemon_endpoint_flux_callback_args_t &args);
/**
 * Handles the priorisea.daemon.formula.list RPC
 *
 * Responds with the requested formulas as an array (all of them when no names
 * are given), or an error if the request is malformed
 *
 * @param args The Flux RPC callback args
 * @return 0 on success, or -1 if the response could not be sent
 */
int ps_daemon_endpoint_flux_formula_list(
    const ps_daemon_endpoint_flux_callback_args_t &args);
/**
 * Handles the priorisea.daemon.formula.update RPC
 *
 * Validates and updates a formula in the database, or responds with an error
 * if the formula is invalid or does not exist
 *
 * @param args The Flux RPC callback args
 * @return 0 on success, or -1 if the response could not be sent
 */
int ps_daemon_endpoint_flux_formula_update(
    const ps_daemon_endpoint_flux_callback_args_t &args);
/**
 * Handles the priorisea.daemon.formula.reset RPC
 *
 * Resets the given formulas to their default values, or responds with an error
 * if the request is malformed
 *
 * @param args The Flux RPC callback args
 * @return 0 on success, or -1 if the response could not be sent
 */
int ps_daemon_endpoint_flux_formula_reset(
    const ps_daemon_endpoint_flux_callback_args_t &args);
