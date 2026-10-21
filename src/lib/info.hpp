// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <jansson.h>

/**
 * Converts an info row to a JSON object.
 *
 * @param info The info row to convert.
 * @return A JSON object representing the info row.
 */
json_t *ps_db_info_to_json(const ps_db_info_t &info);

/**
 * Converts a JSON object to an info row.
 *
 * @param obj The JSON object to convert.
 * @return A result containing the converted info row.
 */
ps_result_t<ps_db_info_t> ps_db_info_from_json(json_t *obj);
