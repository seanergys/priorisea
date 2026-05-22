// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <string>

struct init_formulas_res {
  bool ok;
  std::string message;
};

int run_formula(int argc, char *argv[]);
init_formulas_res init_formulas();
