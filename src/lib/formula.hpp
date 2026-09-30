// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "error.hpp"
#include "types.hpp"
#include <jansson.h>
#include <seanerbus/client/connection.hpp>
#include <string>
#include <vector>

struct ps_db_formula_get_entry_t {
  std::string name;
  ps_result_t<ps_formula_t> result;
};
using ps_db_formulas_get_t = std::vector<ps_db_formula_get_entry_t>;

/**
 * Validates a formula is properly formatted.
 *
 * @param formula The formula to validate.
 * @return True if the formula is valid, false otherwise.
 */
bool ps_validate_formula(const ps_formula_t &formula);

/**
 * Evaluates a formula with the given variables.
 *
 * @param formula The formula to evaluate.
 * @param vars_over The variables to use for evaluation.
 * @return The result of the evaluation.
 */
ps_result_t<double>
ps_eval_formula(const ps_formula_t &formula,
                const std::map<std::string, double> &vars_over);

/**
 * Converts a formula to a JSON object.
 *
 * @param formula The formula to convert.
 * @return The JSON object representing the formula.
 */
json_t *ps_formula_to_json(const ps_formula_t &formula);

/**
 * Converts a JSON object to a formula.
 *
 * @param obj The JSON object to convert.
 * @return A result containing the converted formula.
 */
ps_result_t<ps_formula_t> ps_formula_from_json(json_t *obj);

/**
 * Retrieves formulas from the database by their names.
 *
 * @param names The names of the formulas to retrieve.
 * @return A result containing the retrieved formulas.
 */
ps_result_t<ps_db_formulas_get_t>
ps_db_formulas_get(const std::vector<std::string> &names);

/**
 * Retrieves a formula from the database by its name.
 *
 * @param name The name of the formula to retrieve.
 * @return A result containing the retrieved formula.
 */
ps_result_t<ps_formula_t> ps_db_formula_get(const std::string &name);

/**
 * Sets a formula in the database.
 *
 * @param formula The formula to set.
 * @return A result indicating the success of the operation.
 */
ps_result_t<ps_success_t> ps_db_formula_set(const ps_formula_t &formula);

/**
 * Resets the formulas in the database.
 *
 * @param names The names of the formulas to reset.
 * @return A result indicating the success of the operation.
 */
ps_result_t<ps_success_t>
ps_db_formulas_reset(const std::vector<std::string> &names);

/**
 * Initializes the formulas in the database.
 *
 * @return A result indicating the success of the operation.
 */
ps_result_t<ps_success_t> ps_db_formulas_init();

/**
 * Lists the formulas in the database.
 *
 * @return A result containing the list of formulas.
 */
ps_result_t<ps_db_formulas_get_t> ps_db_formulas_list();

/**
 * Resolves the variables in a formula through the seanerbus.
 *
 * @param formula The formula to resolve.
 * @param jobid The job ID to use.
 * @param seanerbus The seanerbus configuration to use.
 * @return A result containing the resolved variables.
 */
ps_result_t<std::map<std::string, ps_result_t<double>>>
ps_sb_resolve_vars(const ps_formula_t &formula,
                          const flux_jobid_t &jobid,
                          const ps_seanerbus_t &seanerbus);
