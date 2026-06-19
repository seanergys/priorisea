// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "formula.hpp"
#include "error.hpp"
#include "flux.hpp"
#include "tinyexpr.h"
#include "types.hpp"
#include "util.hpp"
#include <sstream>

static const ps_formula_t DEFAULT_JOB_PRIORITY = {
    .name = "job_priority",
    .expr = "prev_priority * 0.5 + user_score * 0.5",
    .vars = {{"prev_priority", 16}, {"user_score", 16}}};

static const ps_formula_t DEFAULT_JOB_SCORE = {
    .name = "job_score",
    .expr = "duration_ratio + timewin_score",
    .vars = {{"timewin_score", 0}, {"duration_ratio", 0}}};

static const std::vector<ps_formula_t> DEFAULT_FORMULAS = {DEFAULT_JOB_PRIORITY,
                                                           DEFAULT_JOB_SCORE};

////

static double min(double a, double b) { return a < b ? a : b; }
static double max(double a, double b) { return a > b ? a : b; }
ps_result_t<double>
ps_eval_formula(const ps_formula_t &formula,
                const std::map<std::string, double> &vars_over) {
  std::map<std::string, double> vars_merged = vars_over;
  for (const auto &[name, value] : formula.vars) {
    if (vars_merged.find(name) != vars_merged.end())
      continue;
    vars_merged[name] = value;
  }

  auto *te_vars =
      (te_variable *)malloc((vars_merged.size() + 2) * sizeof(te_variable));
  int i = 0;

  for (const auto &[name, value] : vars_merged) {
    te_vars[i++] = {name.c_str(), &value};
  }
  te_vars[i++] = {"min", (void *)min, TE_FUNCTION2};
  te_vars[i++] = {"max", (void *)max, TE_FUNCTION2};

  int err = 0;
  te_expr *compiled = te_compile(formula.expr.c_str(), te_vars, i, &err);
  if (!compiled) {
    return ps_error(ps_formula_expr_invalid_t{.expr = formula.expr});
  }
  auto result = te_eval(compiled);
  te_free(compiled);
  return result;
}

bool ps_validate_formula(const ps_formula_t &formula) {
  std::map<std::string, double> vars;
  return (ps_is_ok(ps_eval_formula(formula, vars)));
}

ps_result_t<ps_db_formulas_get_t>
ps_db_formulas_get(const std::vector<std::string> &names) {
  std::vector<std::string> keys;
  for (auto formula_name : names) {
    keys.push_back("priorisea.formula." + formula_name);
  }

  auto lookup_res = ps_flux_kvs_lookup(keys);
  if (!ps_is_ok(lookup_res)) {
    std::stringstream ss;
    ss << "looking up formulas in flux kvs: ";
    for (const auto &key : keys) {
      ss << key << " ";
    }
    return ps_forward_error<ps_db_formulas_get_t>(lookup_res, ss.str());
  }
  std::vector<ps_db_formula_get_entry_t> res;
  for (const auto &lookup_key_res : ps_unwrap(lookup_res)) {
    size_t pos = lookup_key_res.key.rfind('.');
    std::string formula_name = (pos == std::string::npos)
                                   ? lookup_key_res.key
                                   : lookup_key_res.key.substr(pos + 1);

    if (!ps_is_ok(lookup_key_res.value)) {
      res.push_back(
          {.name = formula_name,
           .result = ps_forward_error<ps_formula_t>(
               lookup_key_res.value,
               "looking up formula '" + formula_name + "' in flux kvs")});
      continue;
    }

    auto lkrv = lookup_key_res.value;
    auto lookup_key_unwrap = ps_unwrap(lkrv);

    auto from_res = ps_formula_from_str(lookup_key_unwrap);
    if (!ps_is_ok(from_res)) {
      res.push_back({.name = formula_name,
                     .result = ps_forward_error<ps_formula_t>(
                         from_res, "parsing stored formula '" + formula_name +
                                       "' from flux kvs")});
      continue;
    }

    res.push_back({.name = formula_name, .result = ps_unwrap(from_res)});
  }

  return res;
};

ps_result_t<ps_formula_t> ps_db_formula_get(const std::string &name) {
  auto get_res = ps_db_formulas_get({name});
  if (!ps_is_ok(get_res)) {
    return ps_forward_error<ps_formula_t>(get_res, "getting formula '" + name +
                                                       "' from flux kvs");
  }
  auto get_unwrap = ps_unwrap(get_res);
  return get_unwrap[0].result;
};

ps_result_t<ps_success_t> ps_db_formula_set(const ps_formula_t &formula) {
  if (!ps_validate_formula(formula)) {
    return ps_error(ps_formula_expr_invalid_t{.expr = formula.expr});
  }

  if (formula.name == "job_priority") {
    auto plugin_set_res = ps_flux_plugin_set_formula(formula);
    if (!ps_is_ok(plugin_set_res)) {
      return ps_add_context(plugin_set_res, "sending formula to flux plugin");
    }
  }

  // TODO: if the store is rejected, roll
  // back the priority formula in the flux plugin
  auto kvs_key = "priorisea.formula." + formula.name;
  auto formula_str = ps_formula_to_str(formula);
  auto txn = flux_kvs_txn_create();
  if (!txn) {
    return ps_error(ps_flux_kvs_txn_create_err_t{.msg = strerror(errno)});
  }
  if (flux_kvs_txn_put(txn, 0, kvs_key.c_str(), formula_str.c_str()) < 0) {
    return ps_error(ps_flux_kvs_txn_put_err_t{
        .key = kvs_key, .value = formula_str, .msg = strerror(errno)});
  }
  auto commit_res = ps_flux_kvs_commit(txn);
  if (!ps_is_ok(commit_res)) {
    return ps_forward_error<ps_success_t>(
        commit_res, "committing formula '" + formula.name + "' to flux kvs");
  }
  return ps_success_t{};
}

ps_result_t<ps_success_t>
ps_db_formulas_reset(const std::vector<std::string> &names) {
  for (const auto &name : names) {
    bool found = false;
    ps_formula_t formula;

    for (const auto &default_formula : DEFAULT_FORMULAS) {
      if (name == default_formula.name) {
        formula = default_formula;
        found = true;
        break;
      }
    }

    if (!found) {
      return ps_error(ps_formula_invalid_name_t{.name = name});
    }

    auto update_res = ps_db_formula_set(formula);
    if (!ps_is_ok(update_res)) {
      return ps_forward_error<ps_success_t>(
          update_res, "setting formula '" + name + "' in database");
    }
  }
  return ps_success_t{};
}

ps_result_t<ps_success_t> ps_db_formulas_init() {
  std::vector<std::string> default_formulas_name;
  for (const auto &default_formula : DEFAULT_FORMULAS) {
    default_formulas_name.push_back(default_formula.name);
  }

  auto get_res = ps_db_formulas_get(default_formulas_name);
  if (!ps_is_ok(get_res)) {
    return ps_forward_error<ps_success_t>(
        get_res, "getting default formulas from database");
  }
  auto get_unwrap = ps_unwrap(get_res);

  std::vector<std::string> missing_formulas;
  for (const auto &get_formula_res : get_unwrap) {
    if (!(ps_is_ok(get_formula_res.result))) {
      missing_formulas.push_back(get_formula_res.name);
    }
  }

  auto reset_res = ps_db_formulas_reset(missing_formulas);
  if (!ps_is_ok(reset_res)) {
    std::stringstream ss;
    ss << "resetting missing formulas in database: ";
    for (const auto &formula : missing_formulas) {
      ss << formula << " ";
    }
    return ps_forward_error<ps_success_t>(reset_res, ss.str());
  }

  return ps_success_t{};
}

std::string ps_formula_to_str(const ps_formula_t &formula) {
  std::stringstream ss;
  ss << formula.name << "$" << formula.expr;
  for (auto &[name, value] : formula.vars) {
    ss << "$";
    ss << name << ":" << value;
  }
  return ss.str();
}

ps_result_t<ps_formula_t> ps_formula_from_str(const std::string &formula_str) {
  ps_formula_t res;
  if (formula_str.empty()) {
    return ps_error(ps_str_empty_t{});
  }

  std::stringstream ss(formula_str);
  std::string t;

  if (!getline(ss, t, '$')) {
    return ps_error(ps_formula_name_missing_t{});
  }
  res.name = t;

  if (!getline(ss, t, '$')) {
    return ps_error(ps_formula_expr_missing_t{.name = res.name});
  }
  res.expr = t;

  while (getline(ss, t, '$')) {
    auto pos = t.find(':');
    if (pos == std::string::npos) {
      return ps_error(ps_formula_var_value_missing_t{.name = res.name});
    }

    auto name_str = t.substr(0, pos);
    auto value_str = t.substr(pos + 1);

    double value = 0;
    if (!parse_double(value_str, value)) {
      return ps_error(ps_parse_double_err_t{.double_str = value_str});
    }

    res.vars[name_str] = value;
  }
  return res;
}

ps_result_t<std::vector<ps_db_formula_get_entry_t>> ps_db_formulas_list() {
  auto ls_res = ps_flux_kvs_ls_dir("priorisea.formula");
  if (!ps_is_ok(ls_res)) {
    return ps_forward_error<ps_db_formulas_get_t>(
        ls_res, "listing formulas in flux kvs");
  }
  return ps_db_formulas_get(ps_unwrap(ls_res));
}
