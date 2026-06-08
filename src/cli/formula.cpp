// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "lib/formula.hpp"
#include "formula.hpp"
#include "lib/error.hpp"
#include "lib/types.hpp"
#include "lib/util.hpp"
#include "util.hpp"
#include <iostream>
#include <jansson.h>
#include <sstream>
#include <string.h>
#include <string>
#include <vector>

static void print_formula_json(const std::vector<ps_formula_t> &formulas) {
  auto obj = json_object();
  for (const auto &formula : formulas) {
    auto f_json_obj = json_object();
    json_object_set_new(f_json_obj, "expr", json_string(formula.expr.c_str()));
    auto var_json_obj = json_object();
    for (const auto [var_name, var_value] : formula.vars) {
      json_object_set_new(var_json_obj, var_name.c_str(), json_real(var_value));
    }
    json_object_set_new(f_json_obj, "vars", var_json_obj);
    json_object_set_new(obj, formula.name.c_str(), f_json_obj);
  }
  char *dump = json_dumps(obj, JSON_INDENT(2));
  if (dump) {
    std::cout << dump << std::endl;
    free(dump);
  }
  json_decref(obj);
}

static void print_formula_table(const std::vector<ps_formula_t> &formulas) {
  ps_print_table(
      formulas,
      {{"Name", [](auto &value, auto index) -> auto { return value.name; }},
       {"Formula", [](auto &value, auto index) -> auto { return value.expr; }},
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

////

static void register_init_command(CLI::App *app) {
  auto *init_cmd = app->add_subcommand("init", "Initialize mandatory formulas");
  init_cmd->set_help_flag("-h,--help", "Show help for init command");
  init_cmd->callback([] {
    auto init_res = ps_db_formulas_init();
    if (!ps_is_ok(init_res)) {
      return cli_error(init_res);
    }
  });
}

static void register_list_command(CLI::App *app) {
  auto *list_cmd = app->add_subcommand("list", "List formulas");
  list_cmd->set_help_flag("-h,--help", "Show help for list command");
  struct options {
    bool json;
  };
  auto opts = std::make_shared<options>();
  list_cmd->add_flag("--json", opts->json, "Output in JSON format");
  list_cmd->callback([opts] {
    auto list_formulas_res = ps_db_formulas_list();
    if (!ps_is_ok(list_formulas_res)) {
      return cli_error(list_formulas_res);
    }

    std::vector<ps_formula_t> formulas;
    for (const auto &get_res : ps_unwrap(list_formulas_res)) {
      if (!ps_is_ok(get_res.result)) {
        return cli_error(get_res.result);
      }
      formulas.push_back(ps_unwrap(get_res.result));
    }

    if (opts->json) {
      print_formula_json(formulas);
    } else {
      print_formula_table(formulas);
    }
  });
}

static void register_get_command(CLI::App *app) {
  auto *get_cmd = app->add_subcommand("get", "Get formula values");
  get_cmd->set_help_flag("-h,--help", "Show help for get command");
  struct options {
    std::vector<std::string> formulas_names;
    bool json;
  };
  auto opts = std::make_shared<options>();
  get_cmd->add_option("formulas", opts->formulas_names, "Formulas to get")
      ->required();
  get_cmd->add_flag("--json", opts->json, "Output in JSON format");
  get_cmd->callback([opts] {
    auto get_formula_res = ps_db_formulas_get(opts->formulas_names);
    if (!ps_is_ok(get_formula_res)) {
      return cli_error(get_formula_res);
    }

    std::vector<ps_formula_t> formulas;
    for (const auto &get_formula_res : ps_unwrap(get_formula_res)) {
      if (!ps_is_ok(get_formula_res.result)) {
        return cli_error(get_formula_res.result);
      }
      formulas.push_back(ps_unwrap(get_formula_res.result));
    }

    if (opts->json) {
      print_formula_json(formulas);
    } else {
      print_formula_table(formulas);
    }
  });
}

static void register_set_command(CLI::App *app) {
  auto *set_cmd = app->add_subcommand("set", "Set formula values");
  set_cmd->footer("Example:\n  priorisea formula set job_priority "
                  "\"prev_priority * 2\" --var prev_priority 5 # set "
                  "job_priority formula with a default prev_priority of 5");
  set_cmd->set_help_flag("-h,--help", "Show help for set command");
  struct options {
    std::string name;
    std::string expr;
    std::vector<std::string> vars;
  };
  auto opts = std::make_shared<options>();
  set_cmd->add_option("name", opts->name, "Formulas to set")->required();
  set_cmd->add_option("expr", opts->expr, "Expression to set")->required();
  set_cmd->add_option("-v,--var", opts->vars, "Variables to set")->type_size(2);
  set_cmd->callback([opts] {
    auto formula = ps_formula_t{.name = opts->name, .expr = opts->expr};
    for (size_t i = 0; i < opts->vars.size(); i += 2) {
      auto var_name = opts->vars[i];
      auto value_str = opts->vars[i + 1];
      double value;
      if (!parse_double(value_str, value)) {
        ps_result_t<bool> err =
            ps_error(ps_parse_double_err_t{.double_str = value_str});
        return cli_error(err);
      }
      formula.vars[var_name] = value;
    }
    auto update_res = ps_db_formula_set(formula);
    if (!ps_is_ok(update_res)) {
      return cli_error(update_res);
    }
  });
}

static void register_reset_command(CLI::App *app) {
  auto *reset_cmd =
      app->add_subcommand("reset", "Reset formulas to default values");
  reset_cmd->set_help_flag("-h,--help", "Show help for reset command");
  struct options {
    std::vector<std::string> formulas_names;
  };
  auto opts = std::make_shared<options>();
  reset_cmd->add_option("formulas", opts->formulas_names, "Formulas to reset")
      ->required();
  reset_cmd->callback([opts] {
    auto reset_res = ps_db_formulas_reset(opts->formulas_names);
    if (!ps_is_ok(reset_res)) {
      return cli_error(reset_res);
    }
  });
}

void register_formula_commands(CLI::App *app) {
  auto *formula_cmd = app->add_subcommand("formula", "Manage scoring formulas");
  formula_cmd->require_subcommand(true);
  formula_cmd->set_help_flag("-h,--help", "Show help for formula commands");
  register_init_command(formula_cmd);
  register_list_command(formula_cmd);
  register_get_command(formula_cmd);
  register_set_command(formula_cmd);
  register_reset_command(formula_cmd);
}
