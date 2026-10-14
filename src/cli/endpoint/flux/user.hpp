// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <string>
#include <vector>

ps_result_t<ps_user_t>
ps_cli_endpoint_flux_user_get(const std::string &username);

ps_result_t<std::vector<ps_user_t>>
ps_cli_endpoint_flux_user_list(const std::vector<std::string> &usernames = {});
