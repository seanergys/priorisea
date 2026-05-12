// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "formula.hpp"
#include "score.hpp"
#include <flux/core.h>
#include <iostream>
#include <jansson.h>
#include <string.h>
#include <string>
#include <vector>
#include "lib/table.hpp"
#include "lib/flux.hpp"

struct formula {
  std::string name;
  std::string expr;
};

static const formula DEFAULT_PRIORITY = {.name = "priority", .expr = "0"};
static const formula DEFAULT_SCORE = {.name = "score", .expr = "1"};
static const std::vector<formula> DEFAULT_FORMULAS = {DEFAULT_PRIORITY,
                                                      DEFAULT_SCORE};

enum formula_error {
  INVALID_NAME,
  INVALID_SCORE,
  INVALID_FLUX,
  INVALID_PRIORITY,
  SUCCESS,
};
struct update_formula_result {
  enum formula_error code;
  std::string message;
};
static update_formula_result update_formula_remote(std::string name,
                                                   std::string formula) {
  if (name != "score" && name != "priority") {
    return {
        INVALID_NAME,
        "Invalid formula name. Must be 'score' or 'priority'.",
    };
  }

  if (name == "score" && !validate_score(formula)) {
    return {
        INVALID_SCORE,
        "Invalid score formula.",
    };
  }

  if (name == "priority") {
      auto res = flux_plugin_set_formula(formula);
      if(res.err == FLUX_PLUGIN_ERR_BAD_RESPONSE) {
          return {
              INVALID_PRIORITY,
              res.err_msg
          };
      }
      if(res.err != FLUX_PLUGIN_ERR_OK) {
          return {
              INVALID_FLUX,
              res.err_msg
          };
        }
  }

  // TODO: store formula in the database
  // TODO: if the store is rejected, roll
  // back the priority formula in the flux plugin

  return {
      SUCCESS,
      "",
  };
}

static const std::string USAGE_MSG =
    "Usage: priorisea formula [-h] COMMAND ARGS";
static const std::string TRY_HELP_MSG =
    "Try 'priorisea formula -h' for more information.";
static const std::string HELP_MSG = USAGE_MSG + R"(

Manage priority scoring formulas.
Example: priorisea formula list

COMMAND list and interpretation:
  list: list formula names
  get: get formula values
  set: set a formula
  reset: reset formulas to default values)";

static const std::string LIST_USAGE_MSG =
    "Usage: priorisea formula list [-h] [OPTIONS]";
static const std::string LIST_TRY_HELP_MSG =
    "Try 'priorisea formula list -h' for more information.";
static const std::string LIST_HELP_MSG = LIST_USAGE_MSG + R"(

List the available priority scoring formulas.
Example: priorisea formula list

OPTIONS list and interpretation:
  --json: output in JSON format)";

static int run_list(int argc, char *argv[]) {
  bool json_flag = false;

  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << LIST_HELP_MSG << std::endl;
    return 0;
  }

  for (int i = 0; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--json") {
      json_flag = true;
    } else {
      std::cout << "Error: 'priorisea formula list " << arg
                << "' is not a valid option" << '\n'
                << LIST_USAGE_MSG << '\n'
                << LIST_TRY_HELP_MSG << std::endl;
      return -1;
    }
  }

  // TODO: Retrieve the formula list from the database
  std::vector<formula> results = {
      formula{.name = "priority", .expr = "priority * 2"},
      formula{.name = "score", .expr = "5"}};

  if (json_flag) {
    auto obj = json_array();
    for (const auto &f : results) {
      json_array_append_new(obj, json_string(f.name.c_str()));
    }
    std::cout << json_dumps(obj, JSON_INDENT(2)) << std::endl;
    json_decref(obj);
  } else {
    print_table(results, {{"Name",
                         [](auto &value, auto index) -> auto {
                           return value.name;
                         }}});
  }

  return 0;
}

static const std::string GET_USAGE_MSG =
    "Usage: priorisea formula get [-h] [OPTIONS] NAME [NAME...]";
static const std::string GET_TRY_HELP_MSG =
    "Try 'priorisea formula get -h' for more information.";
static const std::string GET_HELP_MSG = GET_USAGE_MSG + R"(

Retrieve one or more priority scoring formulas from the database.
Example: priorisea formula get priority score

OPTIONS list and interpretation:
  --json: output in JSON format)";

static int run_get(int argc, char *argv[]) {
  bool json_flag = false;
  std::vector<std::string> formulas;

  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << GET_HELP_MSG << std::endl;
    return 0;
  }

  if (argc < 1) {
    std::cout << GET_USAGE_MSG << '\n' << GET_TRY_HELP_MSG << std::endl;
    return -1;
  }

  for (int i = 0; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--json") {
      json_flag = true;
    } else if (!arg.empty() && arg[0] == '-') {
      std::cout << "Error: 'priorisea formula get " << arg
                << "' is not a valid option" << '\n'
                << GET_USAGE_MSG << '\n'
                << GET_TRY_HELP_MSG << std::endl;
      return -1;
    } else {
      formulas.push_back(arg);
    }
  }

  if (formulas.empty()) {
    std::cout << "Error: 'priorisea formula get ...'" << '\n'
              << "  formula name is missing" << '\n'
              << GET_USAGE_MSG << '\n'
              << GET_TRY_HELP_MSG << std::endl;
    return -1;
  }

  // TODO: Retrieve the formulas from the database
  std::vector<formula> results = {
      formula{.name = "priority", .expr = "priority * 2"},
      formula{.name = "score", .expr = "5"}};

  if (json_flag) {
    auto obj = json_object();
    for (const auto &f : results) {
      json_object_set_new(obj, f.name.c_str(), json_string(f.expr.c_str()));
    }
    std::cout << json_dumps(obj, JSON_INDENT(2)) << std::endl;
    json_decref(obj);
  } else {
    print_table(results, {{"Name",
                         [](auto &value, auto index) -> auto {
                           return value.name;
                         }},
                        {"Formula",
                         [](auto &value, auto index) -> auto {
                           return value.expr;
                         }}});
  }
  return 0;
}

static const std::string SET_USAGE_MSG =
    "Usage: priorisea formula set [-h] NAME FORMULA";
static const std::string SET_TRY_HELP_MSG =
    "Try 'priorisea formula set -h' for more information.";
static const std::string SET_HELP_MSG = std::string(SET_USAGE_MSG) + R"(

Set a priority scoring formula.
Example: priorisea formula set priority "score * 2"

OPTIONS list and interpretation:
  --json: output in JSON format)";

static int run_set(int argc, char *argv[]) {
  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << SET_HELP_MSG << std::endl;
    return 0;
  }

  if (argc != 2) {
    std::cout << SET_USAGE_MSG << '\n' << SET_TRY_HELP_MSG << std::endl;
    return -1;
  }

  std::string name = argv[0];
  std::string formula = argv[1];
  auto res = update_formula_remote(name, formula);
  if (res.code != SUCCESS) {
    std::cerr << res.message << std::endl;
    return -1;
  }

  return 0;
}

static const std::string RESET_USAGE_MSG =
    "Usage: priorisea formula reset [-h] NAME [NAME...]";
static const std::string RESET_TRY_HELP_MSG =
    "Try 'priorisea formula reset -h' for more information.";
static const std::string RESET_HELP_MSG = RESET_USAGE_MSG + R"(

Reset one or more priority scoring formulas to their default values.
Example: priorisea formula reset priority score)";
static int run_reset(int argc, char *argv[]) {
  if (argc < 1) {
    std::cout << RESET_USAGE_MSG << '\n' << RESET_TRY_HELP_MSG << std::endl;
    return -1;
  }

  if (strcmp(argv[0], "-h") == 0) {
    std::cout << RESET_HELP_MSG << std::endl;
    return 0;
  }

  for (int i = 0; i < argc; i++) {
    bool found = false;
    std::string name = argv[i];
    std::string formula;

    for (const auto &default_formula : DEFAULT_FORMULAS) {
      if (std::string(argv[i]) == default_formula.name) {
        formula = default_formula.expr;
        found = true;
        break;
      }
    }

    if (!found) {
      std::cout << "Error: 'priorisea formula reset " << argv[i]
                << "' is not a valid formula" << '\n'
                << RESET_USAGE_MSG << '\n'
                << RESET_TRY_HELP_MSG << std::endl;
      return -1;
    }

    auto res = update_formula_remote(name, formula);
    if (res.code != SUCCESS) {
      std::cerr << res.message << std::endl;
      return -1;
    } else {
      std::cout << "Formula " << name << " reset to default: " << formula
                << std::endl;
    }
  }
  return 0;
}

int run_formula(int argc, char *argv[]) {
  if (argc < 1) {
    std::cout << USAGE_MSG << '\n' << TRY_HELP_MSG << std::endl;
    return -1;
  }

  std::string cmd = argv[0];

  if (cmd == "-h") {
    std::cout << HELP_MSG << std::endl;
    return 0;
  } else if (cmd == "list") {
    return run_list(argc - 1, argv + 1);
  } else if (cmd == "get") {
    return run_get(argc - 1, argv + 1);
  } else if (cmd == "set") {
    return run_set(argc - 1, argv + 1);
  } else if (cmd == "reset") {
    return run_reset(argc - 1, argv + 1);
  } else {
    std::cout << "Error: 'priorisea formula " << cmd
              << "' is not a valid command" << std::endl;
    std::cout << USAGE_MSG << std::endl;
    std::cout << TRY_HELP_MSG << std::endl;
    return -1;
  }
}
