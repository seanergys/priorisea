// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "formula.hpp"
#include "util.hpp"
#include <sstream>

std::string formula_to_str(formula formula_obj) {
  std::stringstream ss;
  ss << formula_obj.name << "$" << formula_obj.expr;
  for (auto &[name, value] : formula_obj.vars) {
    ss << "$";
    ss << name << ":" << value;
  }
  return ss.str();
}

formula_from_str_res formula_from_str(std::string formula_str) {
  formula_from_str_res res;
  res.ok = false;

  if (formula_str.empty()) {
    res.message = "formula string is empty";
    return res;
  }

  std::stringstream ss(formula_str);
  std::string t;

  if (!getline(ss, t, '$')) {
    res.ok = false;
    res.message = "name is missing";
    return res;
  }
  res.formula_obj.name = t;

  if (!getline(ss, t, '$')) {
    res.ok = false;
    res.message = "expr is missing";
    return res;
  }
  res.formula_obj.expr = t;

  while (getline(ss, t, '$')) {
    auto pos = t.find(':');
    if (pos == std::string::npos) {
      res.message = "value is missing";
      return res;
    }

    auto name_str = t.substr(0, pos);
    auto value_str = t.substr(pos + 1);

    double value = 0;
    if (!parse_double(value_str, value)) {
      res.message = "value `" + value_str + "` is not a valid double";
      return res;
    }

    res.formula_obj.vars[name_str] = value;
  }

  res.ok = true;
  return res;
}
