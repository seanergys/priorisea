#pragma once
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <src/lib/util.hpp>
#include <string>
#include <vector>

/**
 * Fetch a user by username from the daemon
 *
 * @param username Username to resolve and fetch
 */
ps_result_t<ps_user_t>
ps_cli_endpoint_flux_user_get(const std::string &username);

/**
 * Fetch users from the daemon
 *
 * @param usernames Usernames to fetch, or empty for all
 * @param order The order of the listing, forwarded to the daemon
 */
ps_result_t<std::vector<ps_user_t>>
ps_cli_endpoint_flux_user_list(const std::vector<std::string> &usernames = {},
                               ps_db_order_t order = ps_db_order_t::desc);

/**
 * Update users on the daemon
 *
 * @param usernames Usernames to update, or empty when all is set
 * @param all Whether to update every user
 */
ps_result_t<ps_success_t>
ps_cli_endpoint_flux_user_update(const std::vector<std::string> &usernames,
                                 bool all);
