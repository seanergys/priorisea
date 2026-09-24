// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "error.hpp"
#include "types.hpp"
#include <jansson.h>
#include <string>

/**
 * Converts a decay frame to a JSON object.
 *
 * @param frame The decay frame to convert.
 * @return The JSON object.
 */
json_t *ps_decay_frame_to_json(const ps_decay_frame_t &frame);

/**
 * Converts a JSON object to a decay frame.
 *
 * @param obj The JSON object to convert.
 * @return A result containing the converted decay frame.
 */
ps_result_t<ps_decay_frame_t> ps_decay_frame_from_json(json_t *obj);

/**
 * Converts a decay to a JSON object.
 *
 * @param decay The decay to convert.
 * @return The JSON object.
 */
json_t *ps_decay_to_json(const ps_decay_t &decay);

/**
 * Converts a JSON object to a decay.
 *
 * @param obj The JSON object to convert.
 * @return A result containing the converted decay.
 */
ps_result_t<ps_decay_t> ps_decay_from_json(json_t *obj);

/**
 * Converts a string to a decay.
 *
 * @param expr The string to convert.
 * @return A result containing the converted decay.
 */
ps_result_t<ps_decay_t> ps_decay_from_string(const std::string &expr);

/**
 * Sets the decay in the database.
 *
 * @param decay The decay to set.
 * @return A result indicating success or failure.
 */
ps_result_t<ps_success_t> ps_db_decay_set(const ps_decay_t &decay);

/**
 * Gets the decay from the database.
 *
 * @return A result containing the decay.
 */
ps_result_t<ps_decay_t> ps_db_decay_get();

/**
 * Checks if a decay is stored in the database.
 *
 * @return True if a decay is stored, false otherwise.
 */
bool ps_db_is_decay();

/**
 * Resets the decay in the database.
 *
 * @return A result indicating success or failure.
 */
ps_result_t<ps_success_t> ps_db_decay_reset();

/**
 * Initializes the decay in the database.
 *
 * @return A result indicating success or failure.
 */
ps_result_t<ps_success_t> ps_db_decay_init();
