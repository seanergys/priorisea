// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <map>
#include <string>
#include <vector>

////

struct formula {
  std::string name;
  std::string expr;
  std::map<std::string, double> vars;
};

static const formula DEFAULT_PRIORITY = {
    .name = "priority",
    .expr = "priority * 0.5 + score * 0.5",
    .vars = {{"priority", 16}, {"score", 16}}};

static const formula DEFAULT_SCORE = {.name = "score", .expr = "16"};

static const std::vector<formula> DEFAULT_FORMULAS = {DEFAULT_PRIORITY,
                                                      DEFAULT_SCORE};

////

bool validate_formula_nofun(formula formula_obj);

struct get_formula_res {
  bool ok;
  std::string message;
  std::string name;
  formula formula_obj;
};
get_formula_res get_formula(std::string name);

struct get_formulas_res {
  bool ok;
  std::string message;
  std::vector<get_formula_res> res;
};
get_formulas_res get_formulas(std::vector<std::string> &names);

struct update_formula_res {
  bool ok;
  std::string message;
};
update_formula_res update_formula(formula &formula_obj);

struct reset_formulas_res {
  bool ok;
  std::string message;
};

reset_formulas_res reset_formulas(std::vector<std::string> &names);

struct init_formulas_res {
  bool ok;
  std::string message;
};
init_formulas_res init_formulas();

get_formulas_res get_formulas(std::vector<std::string> &names);

get_formula_res get_formula(std::string name);

std::string formula_to_str(formula formula_obj);

struct formula_from_str_res {
  bool ok;
  std::string message;
  formula formula_obj;
};
formula_from_str_res formula_from_str(std::string formula_str);
