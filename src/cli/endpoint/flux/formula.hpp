// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <string>
#include <vector>

ps_result_t<ps_formula_t>
ps_cli_endpoint_flux_formula_get(const std::string &name);

ps_result_t<std::vector<ps_formula_t>>
ps_cli_endpoint_flux_formula_list(const std::vector<std::string> &names = {});

ps_result_t<ps_success_t>
ps_cli_endpoint_flux_formula_update(const ps_formula_t &formula);

ps_result_t<ps_success_t>
ps_cli_endpoint_flux_formula_reset(const std::vector<std::string> &names);
