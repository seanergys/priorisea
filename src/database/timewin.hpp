// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <sqlite3.h>

using ps_db_timewins_get_t = std::map<std::string, ps_result_t<ps_timewin_t>>;
