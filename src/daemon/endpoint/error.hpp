// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <flux/core.h>
#include <string>

struct ps_plugin_err_t {
  std::string err;
};

std::string ps_error_message(const ps_plugin_err_t &err);
