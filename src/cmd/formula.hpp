// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <string>
#include <vector>
#include "common/formula.hpp"

struct init_formulas_res {
  bool ok;
  std::string message;
};
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

int run_formula(int argc, char *argv[]);
init_formulas_res init_formulas();
get_formulas_res get_formulas(std::vector<std::string> &names);
get_formula_res get_formula(std::string &name);
