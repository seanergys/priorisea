// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "init.hpp"
#include "lib/decay.hpp"
#include "lib/formula.hpp"
#include <cstring>
#include <iostream>
#include <string>

////

static const std::string USAGE_MSG = "Usage: priorisea init [-h]";
static const std::string TRY_HELP_MSG =
    "Try 'priorisea init -h' for more information.";
static const std::string HELP_MSG = USAGE_MSG + R"(

Initialize the default formulas and decay expression to both database and Flux plugin.
This command will override any existing values.
Use it carefully !)";

int run_init(int argc, char *argv[]) {
  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << HELP_MSG << std::endl;
    return 0;
  }
  if (argc > 0) {
    std::cerr << USAGE_MSG << '\n' << TRY_HELP_MSG << std::endl;
    return -1;
  }
  auto init_decay_res = init_decay();
  if (!init_decay_res.ok) {
    std::cerr << "Error: 'priorisea init' went wrong during decay "
                 "initialization"
              << '\n'
              << "  " << init_decay_res.message << '\n'
              << USAGE_MSG << '\n'
              << TRY_HELP_MSG << std::endl;
    return -1;
  }

  auto init_formulas_res = init_formulas();
  if (!init_formulas_res.ok) {
    std::cerr << "Error: 'priorisea init' went wrong during formulas "
                 "initialization"
              << '\n'
              << "  " << init_formulas_res.message << '\n'
              << USAGE_MSG << '\n'
              << TRY_HELP_MSG << std::endl;
    return -1;
  }

  return 0;
}
