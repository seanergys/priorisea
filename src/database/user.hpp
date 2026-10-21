// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <map>
#include <sys/types.h>

using ps_db_sqlite_users_get_t = std::map<uid_t, ps_result_t<ps_user_t>>;
