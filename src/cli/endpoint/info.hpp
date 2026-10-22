// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once

#include "src/lib/types.hpp"

/**
 * Daemon information returned by the info endpoint
 */
struct ps_cli_endpoint_info_get_t {
  int version;
  ps_db_info_t info;
};
