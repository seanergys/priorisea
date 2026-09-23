// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "util.hpp"

void cli_error(const ps_error_t &err) {
  std::cerr << "ERROR: " << ps_error_message(err) << std::endl;
  std::exit(EXIT_FAILURE);
}
