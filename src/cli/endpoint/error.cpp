// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/cli/endpoint/error.hpp"

std::string ps_error_message(const ps_daemon_err_t &err) {
  return "Error reaching the daemon:\n" + err.err;
}
