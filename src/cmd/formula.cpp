// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "formula.hpp"
#include "common/formula.hpp"
#include "common/util.hpp"
#include "lib/flux.hpp"
#include "lib/kvs.hpp"
#include "lib/table.hpp"
#include "tinyexpr.h"
#include <flux/core.h>
#include <iostream>
#include <jansson.h>
#include <sstream>
#include <string.h>
#include <string>
#include <vector>

static const formula DEFAULT_PRIORITY = {
    .name = "priority",
    .expr = "priority * 0.5 + score * 0.5",
    .vars = {{"priority", 16}, {"score", 16}}};
static const formula DEFAULT_SCORE = {.name = "score", .expr = "16"};
static const std::vector<formula> DEFAULT_FORMULAS = {DEFAULT_PRIORITY,
                                                      DEFAULT_SCORE};

////

static bool validate_formula(formula formula_obj) {
  auto *te_vars =
      (te_variable *)malloc(formula_obj.vars.size() * sizeof(te_variable));
  int i = 0;
  for (const auto &[name, value] : formula_obj.vars) {
    te_vars[i++] = {name.c_str(), &value};
  }
  int err = 0;
  te_expr *compiled = te_compile(formula_obj.expr.c_str(), te_vars, i, &err);
  if (!compiled)
    return false;
  te_free(compiled);
  return true;
}

get_formulas_res get_formulas(std::vector<std::string> &names) {
  get_formulas_res res;
  res.ok = false;

  std::vector<std::string> keys;
  for (auto formula_name : names) {
    keys.push_back("priorisea.formula." + formula_name);
  }

  auto lookup_res = kvs_lookup(keys);
  if (lookup_res.err != KVS_ERR_SUCCESS) {
    res.message = "failed to lookup decay string in KVS. Unknown error";
    return res;
  }

  for (const auto &lookup_key_res : lookup_res.res) {
    get_formula_res formula_res;
    formula_res.ok = false;

    size_t pos = lookup_key_res.key.rfind('.');
    std::string formula_name = (pos == std::string::npos)
                                   ? lookup_key_res.key
                                   : lookup_key_res.key.substr(pos + 1);
    formula_res.name = formula_name;

    if (lookup_key_res.err != KVS_ERR_SUCCESS) {
      formula_res.message = "Failed to lookup formula '" + formula_name + "'";
      res.res.push_back(formula_res);
      continue;
    }

    auto from_res = formula_from_str(lookup_key_res.value);
    if (!from_res.ok) {
      formula_res.message = from_res.message;
      res.res.push_back(formula_res);
      continue;
    }

    formula_res.ok = true;
    formula_res.formula_obj = from_res.formula_obj;
    res.res.push_back(formula_res);
  }

  res.ok = true;
  return res;
};

get_formula_res get_formula(std::string &name) {
  get_formula_res res;
  std::vector<std::string> names = {name};
  auto get_res = get_formulas(names);
  if (!get_res.ok) {
    res.ok = false;
    res.message = get_res.message;
    return res;
  }
  return get_res.res[0];
};

struct update_formula_res {
  bool ok;
  std::string message;
};
static update_formula_res update_formula(formula &formula_obj) {
  update_formula_res res;
  auto formula_str = formula_to_str(formula_obj);
  res.ok = false;

  if (!validate_formula(formula_obj)) {
    res.message = "Invalid formula.";
    return res;
  }

  if (formula_obj.name == "priority") {
    auto plugin_set_res = flux_plugin_set_formula(formula_str);
    if (plugin_set_res.err != FLUX_PLUGIN_ERR_OK) {
      res.message = plugin_set_res.err_msg;
      return res;
    }
  }

  // TODO: if the store is rejected, roll
  // back the priority formula in the flux plugin
  auto kvs_key = "priorisea.formula." + formula_obj.name;
  auto txn = flux_kvs_txn_create();
  if (!txn) {
    res.message = "Failed to create KVS transaction.";
    return res;
  }
  if (flux_kvs_txn_put(txn, 0, kvs_key.c_str(), formula_str.c_str()) < 0) {
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
    formula formula_obj;

    for (const auto &default_formula : DEFAULT_FORMULAS) {
      if (name == default_formula.name) {
        formula_obj = default_formula;
        found = true;
        break;
      }
    }

    if (!found) {
      res.message = name + " is not a valid formula";
      return res;
    }

    auto update_res = update_formula(formula_obj);
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

Set a formula used to calculate priority scores.
Example: priorisea formula set priority "score * 2" --var score 10

Be informed that setting the variable mean setting a default value, which can be overridden at runtime.
Example: setting "score" in "priority" formula define the default user score, if not yet defined.

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
