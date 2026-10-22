// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <jansson.h>

/**
 * Converts a formula to a JSON object
 *
 * @param formula The formula to convert
 * @return The JSON object representing the formula
 */
json_t *ps_formula_to_json(const ps_formula_t &formula);

/**
 * Converts a JSON object to a formula
 *
 * @param obj The JSON object to convert
 * @return A result containing the converted formula
 */
ps_result_t<ps_formula_t> ps_formula_from_json(json_t *obj);

/**
 * Validates a formula is properly formatted
 *
 * @param formula The formula to validate
 * @return True if the formula is valid, false otherwise
 */
ps_result_t<ps_success_t> ps_validate_formula(const ps_formula_t &formula);

/**
 * Evaluates a formula with the given variables
 *
 * @param formula The formula to evaluate
 * @param vars_over The variables to use for evaluation
 * @return The result of the evaluation
 */
ps_result_t<double>
ps_eval_formula(const ps_formula_t &formula,
                const std::map<std::string, double> &vars_over);
