// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/cli/endpoint/plugin.hpp"
#include "src/lib/error.hpp"

ps_result_t<ps_success_t> ps_cli_endpoint_flux_plugin_sync();

ps_result_t<ps_cli_endpoint_plugin_debug_t> ps_cli_endpoint_flux_plugin_debug();
