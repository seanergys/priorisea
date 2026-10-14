// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <string>
#include <vector>

ps_result_t<ps_timewin_t>
ps_cli_endpoint_flux_timewin_get(const std::string &name);

ps_result_t<std::vector<ps_timewin_t>>
ps_cli_endpoint_flux_timewin_list(const std::vector<std::string> &names = {});

ps_result_t<ps_success_t>
ps_cli_endpoint_flux_timewin_update(const ps_timewin_t &timewin);

ps_result_t<ps_success_t>
ps_cli_endpoint_flux_timewin_create(const ps_timewin_t &timewin);

ps_result_t<ps_success_t>
ps_cli_endpoint_flux_timewin_delete(const std::vector<std::string> &names);
