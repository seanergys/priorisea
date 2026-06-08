// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "error.hpp"
#include "types.hpp"
#include <string>
#include <vector>

struct ps_db_get_formula_entry_t {
  std::string name;
  ps_result_t<ps_formula_t> result;
};
using ps_db_get_formulas_t = std::vector<ps_db_get_formula_entry_t>;

////

bool ps_validate_formula(const ps_formula_t &formula);

ps_result_t<ps_db_get_formulas_t>
ps_db_formulas_get(const std::vector<std::string> &names);

ps_result_t<ps_formula_t> ps_db_formula_get(const std::string &name);

ps_result_t<ps_success_t> ps_db_formula_set(const ps_formula_t &formula);

ps_result_t<ps_success_t>
ps_db_formulas_reset(const std::vector<std::string> &names);

ps_result_t<ps_success_t> ps_db_formulas_init();

std::string ps_formula_to_str(const ps_formula_t &formula);

ps_result_t<ps_formula_t> ps_formula_from_str(const std::string &formula_str);

ps_result_t<ps_db_get_formulas_t> ps_db_formulas_list();
