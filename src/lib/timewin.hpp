#pragma once
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <jansson.h>
#include <string>

/**
 * Validates a cron expression string
 *
 * @param expr The cron expression string to validate
 * @return A result indicating success or failure
 */
ps_result_t<ps_success_t> ps_validate_cron(const std::string &expr);

/**
 * Validates a time window structure
 *
 * @param timewin The time window structure to validate
 * @return A result indicating success or failure
 */
ps_result_t<ps_success_t> ps_validate_timewin(const ps_timewin_t &timewin);

/**
 * Converts a time window structure to a JSON object
 *
 * @param timewin The time window structure to convert
 * @return A JSON object representing the time window
 */
json_t *ps_timewin_to_json(const ps_timewin_t &timewin);

/**
 * Converts a JSON object to a time window structure
 *
 * @param obj The JSON object to convert
 * @return A result containing the converted time window
 */
ps_result_t<ps_timewin_t> ps_timewin_from_json(json_t *obj);
