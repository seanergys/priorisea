// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "formula.hpp"
#include "lib/flux.hpp"
#include "lib/kvs.hpp"
#include "lib/table.hpp"
#include "score.hpp"
#include <flux/core.h>
#include <iostream>
#include <jansson.h>
#include <string.h>
#include <string>
#include <vector>

struct formula {
  std::string name;
  std::string expr;
};

static const formula DEFAULT_PRIORITY = {.name = "priority", .expr = "0"};
static const formula DEFAULT_SCORE = {.name = "score", .expr = "1"};
static const std::vector<formula> DEFAULT_FORMULAS = {DEFAULT_PRIORITY,
                                                      DEFAULT_SCORE};

////

struct get_formula_res {
  bool ok;
  std::string message;
  std::string name;
  formula formula_obj;
};
struct get_formulas_res {
  bool ok;
  std::string message;
  std::vector<get_formula_res> res;
};
static get_formulas_res get_formulas(std::vector<std::string> &names) {
  get_formulas_res res;
  res.ok = false;
  flux_t *h;
  if (!(h = flux_open(NULL, 0))) {
    res.message = "Failed to connect to Flux.";
    return res;
  }

  for (auto formula_name : names) {
    get_formula_res formula_res;
    formula_res.ok = false;
    formula_res.name = formula_name;
    flux_future_t *f;
    if (!(f = flux_kvs_lookup(h, NULL, 0,
                              ("priorisea.formula." + formula_name).c_str()))) {
      formula_res.message = "Failed to retrieve formula '" + formula_name + "'";
      res.res.push_back(formula_res);
      continue;
    }
    const char *formula_value;
    if (flux_kvs_lookup_get(f, &formula_value) < 0) {
      formula_res.message = "Failed to get formula '" + formula_name + "'";
      res.res.push_back(formula_res);
      flux_future_destroy(f);
      continue;
    }
    formula_res.ok = true;
    formula_res.formula_obj =
        formula{.name = formula_name, .expr = formula_value};
    res.res.push_back(formula_res);
    flux_future_destroy(f);
  }
  flux_close(h);
  res.ok = true;
  return res;
};

struct update_formula_res {
  bool ok;
  std::string message;
};
static update_formula_res update_formula(std::string name, std::string expr) {
  update_formula_res res;
  res.ok = false;
  if (name != "score" && name != "priority") {
    res.message = "Invalid formula name. Must be 'score' or 'priority'.";
    return res;
  }

  if (name == "score" && !validate_score(expr)) {
    res.message = "Invalid score formula.";
    return res;
  }

  if (name == "priority") {
    auto plugin_set_res = flux_plugin_set_formula(expr);
    if (plugin_set_res.err != FLUX_PLUGIN_ERR_OK) {
      res.message = plugin_set_res.err_msg;
      return res;
    }
  }

  // TODO: if the store is rejected, roll
  // back the priority formula in the flux plugin
  auto kvs_key = "priorisea.formula." + name;
  auto txn = flux_kvs_txn_create();
  if (!txn) {
    res.message = "Failed to create KVS transaction.";
    return res;
  }
  if (flux_kvs_txn_put(txn, 0, kvs_key.c_str(), expr.c_str()) < 0) {
    res.message = "Failed to store formula in KVS.";
    return res;
  }
  auto commit_res = kvs_commit(txn);
  if (commit_res != KVS_ERR_SUCCESS) {
    res.message = "Failed to commit formula to KVS.";
    return res;
  }

  res.ok = true;
  return res;
}

struct reset_formulas_res {
  bool ok;
  std::string message;
};

reset_formulas_res reset_formulas(std::vector<std::string> &names) {
  reset_formulas_res res;
  res.ok = false;
  for (const auto &name : names) {
    bool found = false;
    std::string expr;

    for (const auto &default_formula : DEFAULT_FORMULAS) {
      if (name == default_formula.name) {
        expr = default_formula.expr;
        found = true;
        break;
      }
    }

    if (!found) {
      res.message = name + " is not a valid formula";
      return res;
    }

    auto update_res = update_formula(name, expr);
    if (!update_res.ok) {
      res.message = update_res.message;
      return res;
    }
  }
  res.ok = true;
  return res;
}

init_formulas_res init_formulas() {
  init_formulas_res res;
  res.ok = false;

  std::vector<std::string> default_formulas_name;
  for (const auto &default_formula : DEFAULT_FORMULAS) {
    default_formulas_name.push_back(default_formula.name);
  }
  auto get_res = get_formulas(default_formulas_name);
  if (!get_res.ok) {
    res.message = get_res.message;
    return res;
  }

  std::vector<std::string> missing_formulas;
  for (const auto &get_formula_res : get_res.res) {
    if (!get_formula_res.ok) {
      missing_formulas.push_back(get_formula_res.name);
    }
  }

  auto reset_res = reset_formulas(missing_formulas);
  if (!reset_res.ok) {
    res.message = reset_res.message;
    return res;
  }

  res.ok = true;
  return res;
}

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
  std::vector<std::string> formulas_name;
  for (const auto &default_formula : DEFAULT_FORMULAS) {
    formulas_name.push_back(default_formula.name);
  }
  auto get_formula_res = get_formulas(formulas_name);
  if (!get_formula_res.ok) {
    std::cerr << get_formula_res.message << std::endl;
    return -1;
  }

  std::vector<formula> results;
  for (const auto &get_res : get_formula_res.res) {
    if (get_res.ok) {
      results.push_back(get_res.formula_obj);
    } else {
      std::cerr << "Error: " << get_res.message << std::endl;
    }
  }

  if (json_flag) {
    auto obj = json_array();
    for (const auto &f : results) {
      json_array_append_new(obj, json_string(f.name.c_str()));
    }
    std::cout << json_dumps(obj, JSON_INDENT(2)) << std::endl;
    json_decref(obj);
  } else {
    print_table(results, {{"Name", [](auto &value, auto index) -> auto {
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
      json_object_set_new(obj, f.name.c_str(), json_string(f.expr.c_str()));
    }
    std::cout << json_dumps(obj, JSON_INDENT(2)) << std::endl;
    json_decref(obj);
  } else {
    print_table(
        results,
        {{"Name", [](auto &value, auto index) -> auto { return value.name; }},
         {"Formula",
          [](auto &value, auto index) -> auto { return value.expr; }}});
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
  std::string expr = argv[1];
  auto update_res = update_formula(name, expr);
  if (!update_res.ok) {
    std::cerr << update_res.message << std::endl;
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

Manage priority scoring formulas.
Example: priorisea formula list

COMMAND list and interpretation:
  init: initialize default formulas
  list: list formula names
  get: get formula values
  set: set a formula
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
