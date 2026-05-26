// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <map>
#include <string>
struct formula {
  std::string name;
  std::string expr;
  std::map<std::string, double> vars;
};
struct formula_from_str_res {
  bool ok;
  std::string message;
  formula formula_obj;
};

std::string formula_to_str(formula formula_obj);
formula_from_str_res formula_from_str(std::string formula_str);
