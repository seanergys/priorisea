// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "util.hpp"

void cli_error(ps_error_t &err) {
  std::cerr << ps_error_message(err) << std::endl;
  std::exit(EXIT_FAILURE);
}
