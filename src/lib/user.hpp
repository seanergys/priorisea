// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <jansson.h>

/**
 * Converts a user to a JSON object.
 *
 * @param user The user to convert.
 * @return A JSON object representing the user.
 */
json_t *ps_user_to_json(const ps_user_t &user);

/**
 * Converts a JSON object to a user.
 *
 * @param obj The JSON object to convert.
 * @return A result containing the converted user.
 */
ps_result_t<ps_user_t> ps_user_from_json(json_t *obj);
