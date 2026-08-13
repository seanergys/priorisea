// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "error.hpp"
#include "types.hpp"
#include <jansson.h>
#include <string>
#include <vector>

struct ps_db_formula_get_entry_t {
  std::string name;
  ps_result_t<ps_formula_t> result;
};
using ps_db_formulas_get_t = std::vector<ps_db_formula_get_entry_t>;

////

bool ps_validate_formula(const ps_formula_t &formula);

ps_result_t<double>
ps_eval_formula(const ps_formula_t &formula,
                const std::map<std::string, double> &vars_over);

json_t *ps_formula_to_json(const ps_formula_t &formula);
ps_formula_t ps_formula_from_json(json_t *obj);

ps_result_t<ps_db_formulas_get_t>
ps_db_formulas_get(const std::vector<std::string> &names);

ps_result_t<ps_formula_t> ps_db_formula_get(const std::string &name);

ps_result_t<ps_success_t> ps_db_formula_set(const ps_formula_t &formula);

ps_result_t<ps_success_t>
ps_db_formulas_reset(const std::vector<std::string> &names);

ps_result_t<ps_success_t> ps_db_formulas_init();

ps_result_t<ps_db_formulas_get_t> ps_db_formulas_list();
