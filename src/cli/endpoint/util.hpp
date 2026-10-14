// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include <flux/core.h>

ps_error_kind_t ps_cli_endpoint_unpack_err(flux_future_t *f);
