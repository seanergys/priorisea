// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/types.hpp"
#include <sys/types.h>

struct ps_cli_endpoint_plugin_debug_t {
  ps_formula_t formula;
  std::map<uid_t, double> scores;
};
