// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "error.hpp"
#include "types.hpp"
#include <jansson.h>
#include <optional>
#include <string>

std::string ps_config_default_path();
ps_result_t<ps_seanerbus_t>
ps_get_seanerbus(const std::optional<json_t *> &config_json);
ps_result_t<ps_config_t> ps_config_load(const std::optional<std::string> &path);

json_t *ps_config_to_json(const ps_config_t &config);
