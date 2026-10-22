#pragma once
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <string>
#include <vector>

/**
 * Fetch a single formula by name from the daemon
 *
 * @param name Name of the formula to fetch
 */
ps_result_t<ps_formula_t>
ps_cli_endpoint_flux_formula_get(const std::string &name);

/**
 * Fetch formulas from the daemon
 *
 * @param names Names to fetch, or empty for all
 */
ps_result_t<std::vector<ps_formula_t>>
ps_cli_endpoint_flux_formula_list(const std::vector<std::string> &names = {});

/**
 * Update a formula on the daemon
 *
 * @param formula Formula with the new values to store
 */
ps_result_t<ps_success_t>
ps_cli_endpoint_flux_formula_update(const ps_formula_t &formula);

/**
 * Reset formulas to their default values on the daemon
 *
 * @param names Names of the formulas to reset
 */
ps_result_t<ps_success_t>
ps_cli_endpoint_flux_formula_reset(const std::vector<std::string> &names);
