// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "lib/formula.hpp"
#include "lib/util.hpp"
#include <iostream>
#include <jansson.h>
#include <sstream>
#include <string.h>
#include <string>
#include <vector>

////

static const std::string INIT_USAGE_MSG = "Usage: priorisea formula init [-h]";
static const std::string INIT_TRY_HELP_MSG =
    "Try 'priorisea formula init -h' for more information.";
static const std::string INIT_HELP_MSG = INIT_USAGE_MSG + R"(

Initialize the mandatory formulas for scoring.
If the mandatory formulas are already stored, this command will not do anything.)";

static int run_init(int argc, char *argv[]) {
  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << INIT_HELP_MSG << std::endl;
    return 0;
  }
  if (argc > 0) {
    std::cerr << INIT_USAGE_MSG << '\n' << INIT_TRY_HELP_MSG << std::endl;
    return -1;
  }

  auto init_res = init_formulas();
  if (!init_res.ok) {
    std::cerr << "Error: 'priorisea decay init" << '\n'
              << "  " << init_res.message << '\n'
              << INIT_USAGE_MSG << '\n'
              << INIT_TRY_HELP_MSG << std::endl;
    return -1;
  }

  return 0;
}

static const std::string LIST_USAGE_MSG =
    "Usage: priorisea formula list [-h] [OPTIONS]";
static const std::string LIST_TRY_HELP_MSG =
    "Try 'priorisea formula list -h' for more information.";
static const std::string LIST_HELP_MSG = LIST_USAGE_MSG + R"(

List the available scoring formulas.
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

  auto list_formulas_res = list_formulas();
  if (!list_formulas_res.ok) {
    std::cout << "Error: 'priorisea formula list ..." << "  "
              << list_formulas_res.message << '\n'
              << LIST_USAGE_MSG << '\n'
              << LIST_TRY_HELP_MSG << std::endl;
    return -1;
  }

  std::vector<formula> results;
  for (const auto &get_res : list_formulas_res.res) {
    if (get_res.ok) {
      results.push_back(get_res.formula_obj);
    } else {
      std::cerr << "Error: " << get_res.message << std::endl;
    }
  }

  if (json_flag) {
    auto obj = json_object();
    for (const auto &f : results) {
        auto f_json_obj = json_object();
        json_object_set_new(f_json_obj, "expr", json_string(f.expr.c_str()));
        auto var_json_obj = json_object();
        for(const auto [var_name, var_value] : f.vars) {
            json_object_set_new(var_json_obj, var_name.c_str(), json_string(std::to_string(var_value).c_str()));
        }
        json_object_set_new(f_json_obj, "vars", var_json_obj);
        json_object_set_new(obj, f.name.c_str(), f_json_obj);
    }
    std::cout << json_dumps(obj, JSON_INDENT(2)) << std::endl;
    json_decref(obj);
  } else {
    print_table(
        results,
        {{"Name", [](auto &value, auto index) -> auto { return value.name; }},
         {"Formula",
          [](auto &value, auto index) -> auto { return value.expr; }},
         {"Vars", [](auto &value, auto index) -> auto {
            std::stringstream ss;
            bool first = true;
            for (const auto &[name, value] : value.vars) {
              if (!first) {
                ss << ", ";
              } else {
                first = false;
              }
              ss << name << "=" << value;
            }
            return ss.str();
          }}});
  }

  return 0;
}

static const std::string GET_USAGE_MSG =
    "Usage: priorisea formula get [-h] [OPTIONS] NAME [NAME...]";
static const std::string GET_TRY_HELP_MSG =
    "Try 'priorisea formula get -h' for more information.";
static const std::string GET_HELP_MSG = GET_USAGE_MSG + R"(

Retrieve one or more scoring formulas from the database.
Example: priorisea formula get job_priority job_score

OPTIONS list and interpretation:
  --json: output in JSON format)";

static int run_get(int argc, char *argv[]) {
  bool json_flag = false;
  std::vector<std::string> formulas_names;

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
      formulas_names.push_back(arg);
    }
  }

  if (formulas_names.empty()) {
    std::cout << "Error: 'priorisea formula get ...'" << '\n'
              << "  formula name is missing" << '\n'
              << GET_USAGE_MSG << '\n'
              << GET_TRY_HELP_MSG << std::endl;
    return -1;
  }

  auto get_formula_res = get_formulas(formulas_names);
  if (!get_formula_res.ok) {
    std::cerr << get_formula_res.message << std::endl;
    return -1;
  }
  std::vector<formula> results;
  for (const auto &get_formula_res : get_formula_res.res) {
    if (get_formula_res.ok) {
      results.push_back(get_formula_res.formula_obj);
    } else {
      std::cerr << "Error: " << get_formula_res.message << std::endl;
    }
  }

  if (json_flag) {
      auto obj = json_object();
      for (const auto &f : results) {
          auto f_json_obj = json_object();
          json_object_set_new(f_json_obj, "expr", json_string(f.expr.c_str()));
          auto var_json_obj = json_object();
          for(const auto [var_name, var_value] : f.vars) {
              json_object_set_new(var_json_obj, var_name.c_str(), json_string(std::to_string(var_value).c_str()));
          }
          json_object_set_new(f_json_obj, "vars", var_json_obj);
          json_object_set_new(obj, f.name.c_str(), f_json_obj);
      }
      std::cout << json_dumps(obj, JSON_INDENT(2)) << std::endl;
      json_decref(obj);
  } else {
    print_table(
        results,
        {{"Name", [](auto &value, auto index) -> auto { return value.name; }},
         {"Formula",
          [](auto &value, auto index) -> auto { return value.expr; }},
         {"Vars", [](auto &value, auto index) -> auto {
            std::stringstream ss;
            bool first = true;
            for (const auto &[name, value] : value.vars) {
              if (!first) {
                ss << ", ";
              } else {
                first = false;
              }
              ss << name << "=" << value;
            }
            return ss.str();
          }}});
  }
  return 0;
}

static const std::string SET_USAGE_MSG =
    "Usage: priorisea formula set [-h] NAME \"EXPR\" [--var NAME VALUE]...";
static const std::string SET_TRY_HELP_MSG =
    "Try 'priorisea formula set -h' for more information.";
static const std::string SET_HELP_MSG = std::string(SET_USAGE_MSG) + R"(

Set a formula used to calculate scores.
Example: priorisea formula set job_priority "user_score * 2" --var user_score 10

Be informed that setting the variable mean setting a default value, which can be overridden at runtime.
Example: setting "user_score" in "job_priority" formula define the default user score, if not yet defined.

OPTIONS list and interpretation:
  --var:  a variable name and default value to set in the formula)";

static int run_set(int argc, char *argv[]) {
  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << SET_HELP_MSG << std::endl;
    return 0;
  }

  if (argc < 2) {
    std::cout << SET_USAGE_MSG << '\n' << SET_TRY_HELP_MSG << std::endl;
    return -1;
  }

  formula formula_obj = {
      .name = argv[0],
      .expr = argv[1],
  };

  for (int i = 2; i < argc;) {
    std::string arg = argv[i];
    if (arg == "--var") {
      if (i + 2 >= argc) {
        std::cerr << "Error: 'priorisea formula set ..." << '\n'
                  << "  '--var' requires a variable name and value" << '\n'
                  << SET_USAGE_MSG << '\n'
                  << SET_TRY_HELP_MSG << std::endl;
        return -1;
      }
      auto var_name = argv[i + 1];
      auto var_value_str = argv[i + 2];
      double var_value;
      if (!parse_double(var_value_str, var_value)) {
        std::cerr << "Error: 'priorisea formula set ..." << '\n'
                  << "  '" << var_value_str << "' is not a valid double" << '\n'
                  << SET_USAGE_MSG << '\n'
                  << SET_TRY_HELP_MSG << std::endl;
        return -1;
      }
      formula_obj.vars[var_name] = var_value;
      i += 3;
    } else {
      std::cerr << "Error: 'priorisea formula set ..." << '\n'
                << "  '" << arg << "' is not a valid option" << '\n'
                << SET_USAGE_MSG << '\n'
                << SET_TRY_HELP_MSG << std::endl;
      return -1;
    }
  }

  auto update_res = update_formula(formula_obj);
  if (!update_res.ok) {
    std::cerr << "Error: 'priorisea formula set ..." << '\n'
              << "  " << update_res.message << '\n'
              << SET_USAGE_MSG << '\n'
              << SET_TRY_HELP_MSG << std::endl;
    return -1;
  }

  return 0;
}

static const std::string RESET_USAGE_MSG =
    "Usage: priorisea formula reset [-h] NAME [NAME...]";
static const std::string RESET_TRY_HELP_MSG =
    "Try 'priorisea formula reset -h' for more information.";
static const std::string RESET_HELP_MSG = RESET_USAGE_MSG + R"(

Reset one or more scoring formulas to their default values.
Example: priorisea formula reset job_priority job_score)";
static int run_reset(int argc, char *argv[]) {
  if (argc < 1) {
    std::cout << RESET_USAGE_MSG << '\n' << RESET_TRY_HELP_MSG << std::endl;
    return -1;
  }

  if (strcmp(argv[0], "-h") == 0) {
    std::cout << RESET_HELP_MSG << std::endl;
    return 0;
  }

  std::vector<std::string> formulas_name;
  for (int i = 0; i < argc; i++) {
    formulas_name.push_back(argv[i]);
  }
  auto reset_res = reset_formulas(formulas_name);
  if (!reset_res.ok) {
    std::cerr << "Error: 'priorisea formula reset ..." << '\n'
              << "  " << reset_res.message << '\n'
              << RESET_USAGE_MSG << '\n'
              << RESET_TRY_HELP_MSG << std::endl;
    return -1;
  }
  return 0;
}

static const std::string USAGE_MSG =
    "Usage: priorisea formula [-h] COMMAND ARGS";
static const std::string TRY_HELP_MSG =
    "Try 'priorisea formula -h' for more information.";
static const std::string HELP_MSG = USAGE_MSG + R"(

Manage scoring formulas.
Example: priorisea formula list

COMMAND list and interpretation:
  init : initialize default formulas
  list : list formula names
  get  : get formula values
  set  : set a formula
  reset: reset formulas to default values)";
int run_formula(int argc, char *argv[]) {
  if (argc < 1) {
    std::cout << USAGE_MSG << '\n' << TRY_HELP_MSG << std::endl;
    return -1;
  }

  std::string cmd = argv[0];

  if (cmd == "-h") {
    std::cout << HELP_MSG << std::endl;
    return 0;
  } else if (cmd == "init") {
    return run_init(argc - 1, argv + 1);
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
