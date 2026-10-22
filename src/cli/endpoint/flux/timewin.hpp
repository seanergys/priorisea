// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <string>
#include <vector>

/**
 * Fetch a single timewin by name from the daemon
 *
 * @param name Name of the timewin to fetch
 */
ps_result_t<ps_timewin_t>
ps_cli_endpoint_flux_timewin_get(const std::string &name);

/**
 * Fetch timewins from the daemon
 *
 * @param names Names to fetch, or empty for all
 */
ps_result_t<std::vector<ps_timewin_t>>
ps_cli_endpoint_flux_timewin_list(const std::vector<std::string> &names = {});

/**
 * Update a timewin on the daemon
 *
 * @param timewin Timewin with the new values to store
 */
ps_result_t<ps_success_t>
ps_cli_endpoint_flux_timewin_update(const ps_timewin_t &timewin);

/**
 * Create a timewin on the daemon
 *
 * @param timewin Timewin to create
 */
ps_result_t<ps_success_t>
ps_cli_endpoint_flux_timewin_create(const ps_timewin_t &timewin);

/**
 * Delete timewins from the daemon
 *
 * @param names Names of the timewins to delete
 */
ps_result_t<ps_success_t>
ps_cli_endpoint_flux_timewin_delete(const std::vector<std::string> &names);
