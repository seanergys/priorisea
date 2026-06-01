// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "decay.hpp"
#include "formula.hpp"
#include "init.hpp"
#include "score.hpp"
#include "timewin.hpp"
#include <iostream>

static const std::string USAGE_MSG = "Usage: priorisea [-h] COMMAND ARGS";
static const std::string TRY_HELP_MSG =
    "Try 'priorisea -h' for more information.";
static const std::string HELP_MSG = USAGE_MSG + R"(

Interact with Seanergys' priority plugin and database to manage priorities.
Example: priorisea formula list

COMMAND list and interpretation:
  init   : initialize default formulas and decay expression
  decay  : manage priority decay
  formula: manage priority formulas
  timewin: manage priority time windows
  score  : manage priority scores)";

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << USAGE_MSG << std::endl;
    std::cout << TRY_HELP_MSG << std::endl;
    return -1;
  }

  std::string cmd = argv[1];
  if (cmd == "-h") {
    std::cout << HELP_MSG << std::endl;
    return 0;
  } else if (cmd == "init") {
    return run_init(argc - 2, argv + 2);
  } else if (cmd == "formula") {
    return run_formula(argc - 2, argv + 2);
  } else if (cmd == "timewin") {
    return run_time(argc - 2, argv + 2);
  } else if (cmd == "score") {
    return run_score(argc - 2, argv + 2);
  } else if (cmd == "decay") {
    return run_decay(argc - 2, argv + 2);
  } else {
    std::cout << "Error: 'priorisea " << cmd << "' is not a valid command"
              << std::endl;
    std::cout << USAGE_MSG << std::endl;
    std::cout << TRY_HELP_MSG << std::endl;
    return -1;
  }
}
