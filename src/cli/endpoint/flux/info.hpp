// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/cli/endpoint/info.hpp"
#include "src/lib/error.hpp"

/**
 * Fetch the daemon information
 * including the protocol version and stored periods
 */
ps_result_t<ps_cli_endpoint_info_get_t> ps_cli_endpoint_info_get();
