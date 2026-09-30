// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "error.hpp"
#include "types.hpp"
#include <jansson.h>
#include <optional>
#include <string>

/**
 * Returns the default path for the configuration file.
 *
 * @return The default path as a string.
 */
std::string ps_config_default_path();

/**
 * Parses the Seanerbus configuration from a JSON object.
 *
 * @param config_json The JSON object containing the configuration.
 * @return A result containing the parsed Seanerbus configuration.
 */
ps_result_t<ps_seanerbus_t>
ps_get_seanerbus(const std::optional<json_t *> &config_json);

/**
 * Loads the configuration from a file.
 *
 * @param path The path to the configuration file.
 * @return A result containing the loaded configuration.
 */
ps_result_t<ps_config_t> ps_config_load(const std::optional<std::string> &path);

/**
 * Converts the configuration to a JSON object.
 *
 * @param config The configuration to convert.
 * @return A JSON object representing the configuration.
 */
json_t *ps_config_to_json(const ps_config_t &config);

/**
 * Converts a Seanerbus configuration to a JSON object.
 *
 * @param seanerbus The Seanerbus configuration to convert.
 * @return A JSON object representing the Seanerbus.
 */
json_t *ps_seanerbus_to_json(const ps_seanerbus_t &seanerbus);

/**
 * Converts a JSON object to a Seanerbus configuration.
 *
 * @param obj The JSON object to convert.
 * @return A result containing the Seanerbus configuration.
 */
ps_result_t<ps_seanerbus_t> ps_seanerbus_from_json(json_t *obj);
