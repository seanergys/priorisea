// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"

ps_result_t<ps_decay_t> ps_cli_endpoint_flux_decay_get();

ps_result_t<ps_success_t> ps_cli_endpoint_flux_decay_reset();

ps_result_t<ps_success_t>
ps_cli_endpoint_flux_decay_update(const ps_decay_t &decay);
