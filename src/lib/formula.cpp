// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "formula.hpp"
#include "flux.hpp"
#include "score.hpp"
#include "tinyexpr.h"
#include "util.hpp"
#include <sstream>

bool validate_formula_nofun(formula formula_obj) {
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
  if (lookup_res.err != FLUX_KVS_ERR_SUCCESS) {
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

    if (lookup_key_res.err != FLUX_KVS_ERR_SUCCESS) {
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

get_formula_res get_formula(std::string name) {
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

update_formula_res update_formula(formula &formula_obj) {
  update_formula_res res;
  auto formula_str = formula_to_str(formula_obj);
  res.ok = false;

  if (formula_obj.name == "job_score" && !validate_formula_job_score(formula_obj)) {
    res.message = "Invalid formula.";
    return res;
  }

  if (formula_obj.name == "job_priority") {
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
  if (commit_res != FLUX_KVS_ERR_SUCCESS) {
    res.message = "Failed to commit formula to KVS.";
    return res;
  }

  res.ok = true;
  return res;
}

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

get_formulas_res list_formulas() {
  get_formulas_res res;
  res.ok = false;

  auto ls_res = flux_kvs_ls_dir("priorisea.formula");
  if (!ls_res.ok) {
    res.message = ls_res.message;
    return res;
  }

  return get_formulas(ls_res.keys);
}
