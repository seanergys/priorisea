// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include <flux/core.h>
#include <sqlite3.h>

struct ps_daemon_endpoint_flux_callback_args_t {
  flux_t *h;
  const flux_msg_t *msg;
  sqlite3 *db;
  std::string seanerbus_url;
};

ps_result_t<ps_success_t> ps_daemon_endpoint_flux_run(sqlite3 *db,
                                                      std::string seanerbus);
