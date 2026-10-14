// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <map>

using ps_db_sqlite_users_get_t = std::map<std::string, ps_result_t<ps_user_t>>;
