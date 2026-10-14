// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/database/formula.hpp"
#include "src/database/error.hpp"
#include "src/database/sqlite/formula.hpp"
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"

static const ps_formula_t DEFAULT_JOB_PRIORITY = {
    .name = "job_priority",
    .expr = "<var:prev_priority> * 0.5 + <var:user_score> * 0.5",
    .defaults = {{"var:prev_priority", 16}, {"var:user_score", 16}}};

static const ps_formula_t DEFAULT_JOB_SCORE = {
    .name = "job_score",
    .expr = "<var:duration_ratio> + <var:timewin_score>",
    .defaults = {{"var:timewin_score", 0}, {"var:duration_ratio", 0}}};

static const std::vector<ps_formula_t> DEFAULT_FORMULAS = {DEFAULT_JOB_PRIORITY,
                                                           DEFAULT_JOB_SCORE};

ps_result_t<ps_success_t>
ps_db_formulas_reset(sqlite3 *db, const std::vector<std::string> &names) {
  std::map<std::string, ps_formula_t> default_formulas_map;
  for (const auto &default_formula : DEFAULT_FORMULAS) {
    default_formulas_map[default_formula.name] = default_formula;
  }

  // Resolve every requested name to its default formula. If any name is not a
  // valid default formula, reset nothing.
  std::vector<ps_formula_t> defaults;
  for (const auto &name : names) {
    auto it = default_formulas_map.find(name);
    if (it == default_formulas_map.end()) {
      return ps_error(ps_db_not_found_t{.key = "formula." + name});
    }
    defaults.push_back(it->second);
  }

  return ps_db_sqlite_formulas_set(db, defaults);
}

ps_result_t<ps_success_t> ps_db_formulas_init(sqlite3 *db) {

  std::vector<std::string> default_formulas_name;
  for (const auto &default_formula : DEFAULT_FORMULAS) {
    default_formulas_name.push_back(default_formula.name);
  }

  auto get_res = ps_db_sqlite_formulas_get(db, default_formulas_name);
  if (!ps_is_ok(get_res)) {
    return ps_forward_error<ps_success_t>(get_res);
  }

  // If a formula is missing, we reset it to the default value
  auto get_val = ps_unwrap(get_res);
  std::vector<std::string> missing_formulas;
  for (const auto &[name, res] : get_val) {
    if (!(ps_is_ok(res))) {
      missing_formulas.push_back(name);
    }
  }

  auto reset_res = ps_db_formulas_reset(db, missing_formulas);
  if (!ps_is_ok(reset_res)) {
    return ps_forward_error<ps_success_t>(reset_res);
  }
  return ps_success_t{};
}
