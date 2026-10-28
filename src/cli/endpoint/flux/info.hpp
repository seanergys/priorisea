#pragma once
#include "src/cli/endpoint/info.hpp"
#include "src/lib/error.hpp"

/**
 * Fetch the daemon information
 * including the protocol version and stored periods
 */
ps_result_t<ps_cli_endpoint_info_get_t> ps_cli_endpoint_info_get();

/**
 * Update the user_period in
 * the database info table
 */
ps_result_t<ps_success_t>
ps_cli_endpoint_flux_user_period_update(double user_period);
