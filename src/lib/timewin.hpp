// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
// #include "croncpp.h"
#include "error.hpp"
#include "types.hpp"
#include <flux/core.h>
#include <jansson.h>
#include <string>
#include <vector>

struct ps_db_timewin_get_entry_t {
  std::string name;
  ps_result_t<ps_timewin_t> result;
};
using ps_db_timewins_get_t = std::vector<ps_db_timewin_get_entry_t>;
struct ps_db_timewin_del_entry_t {
  std::string name;
  ps_result_t<ps_success_t> result;
};
using ps_db_timewins_del_t = std::vector<ps_db_timewin_del_entry_t>;

/**
 * Validates a cron expression string.
 *
 * @param expr The cron expression string to validate.
 * @return A result indicating success or failure.
 */
ps_result_t<ps_success_t> ps_validate_cron(const std::string &expr);

/**
 * Validates a time window structure.
 *
 * @param timewin The time window structure to validate.
 * @return A result indicating success or failure.
 */
ps_result_t<ps_success_t> ps_validate_timewin(const ps_timewin_t &timewin);

/**
 * Converts a time window structure to a JSON object.
 *
 * @param timewin The time window structure to convert.
 * @return A JSON object representing the time window.
 */
json_t *ps_timewin_to_json(const ps_timewin_t &timewin);

/**
 * Converts a JSON object to a time window structure.
 *
 * @param obj The JSON object to convert.
 * @return A result containing the converted time window.
 */
ps_result_t<ps_timewin_t> ps_timewin_from_json(json_t *obj);

/**
 * Sets a time window in the database.
 *
 * @param timewin The time window structure to set in the database.
 * @return A result indicating success or failure.
 */
ps_result_t<ps_success_t> ps_db_timewin_set(const ps_timewin_t &timewin);

/**
 * Retrieves multiple time windows from the database based on their names.
 *
 * @param names A vector of time window names to retrieve.
 * @return A result containing the retrieved time windows.
 */
ps_result_t<ps_db_timewins_get_t>
ps_db_timewins_get(const std::vector<std::string> &names);

/**
 * Retrieves a single time window from the database based on its name.
 *
 * @param name The name of the time window to retrieve.
 * @return A result containing the retrieved time window.
 */
ps_result_t<ps_timewin_t> ps_db_timewin_get(const std::string &name);

/**
 * Deletes multiple time windows from the database based on their names.
 *
 * @param names A vector of time window names to delete.
 * @return A result indicating success or failure for each deletion.
 */
ps_result_t<ps_db_timewins_del_t>
ps_db_timewins_del(const std::vector<std::string> &names);

/**
 * Deletes a single time window from the database based on its name.
 *
 * @param name The name of the time window to delete.
 * @return A result indicating success or failure.
 */
ps_result_t<ps_success_t> ps_db_timewin_del(const std::string &name);

/**
 * Lists all time windows in the database.
 *
 * @return A result containing the list of time windows.
 */
ps_result_t<ps_db_timewins_get_t> ps_db_timewins_list();
