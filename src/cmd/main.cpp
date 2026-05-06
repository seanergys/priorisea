#include "decay.hpp"
#include "formula.hpp"
#include "score.hpp"
#include "time.hpp"
#include <iostream>

static const std::string USAGE_MSG = "Usage: priorisea [-h] COMMAND ARGS";
static const std::string HELP_MSG = "Try 'priorisea -h' for more information.";
static int run_help(int argc, char *argv[]) {
  std::cout << USAGE_MSG << std::endl;
  std::cout << "Interact with Seanergys' priority plugin and database to "
               "manage priorities."
            << std::endl;
  std::cout << "Example: priorisea formula list" << std::endl;
  std::cout << std::endl;
  std::cout << "COMMAND list and interpretation:" << std::endl;
  std::cout << "  formula: manage priority formulas" << std::endl;
  std::cout << "  time:    manage priority time" << std::endl;
  std::cout << "  score:   manage priority scores" << std::endl;
  std::cout << "  decay:   manage priority decay" << std::endl;
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << USAGE_MSG << std::endl;
    std::cout << HELP_MSG << std::endl;
    return 1;
  }

  std::string cmd = argv[1];
  if (cmd == "-h") {
    return run_help(argc - 2, argv + 2);
  } else if (cmd == "formula") {
    return run_formula(argc - 2, argv + 2);
  } else if (cmd == "time") {
    return run_time(argc - 2, argv + 2);
  } else if (cmd == "score") {
    return run_score(argc - 2, argv + 2);
  } else if (cmd == "decay") {
    return run_decay(argc - 2, argv + 2);
  } else {
    std::cout << "Error: 'priorisea " << cmd << "' is not a valid command"
              << std::endl;
    std::cout << USAGE_MSG << std::endl;
    std::cout << HELP_MSG << std::endl;
    return -1;
  }
}
