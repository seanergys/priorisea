// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include <flux/core.h>
#include <sqlite3.h>

/**
 * Context passed to every Flux RPC callback
 */
struct ps_daemon_endpoint_flux_callback_args_t {
  flux_t *h;
  const flux_msg_t *msg;
  sqlite3 *db;
  std::string seanerbus_url;
};

/**
 * Opens Flux, registers the priorisea service, installs every RPC message
 * handler, and runs the reactor loop until the daemon stops
 *
 * @param db The open database
 * @param seanerbus The SeanerBus URL
 * @return A result indicating success or failure
 */
ps_result_t<ps_success_t> ps_daemon_endpoint_flux_run(sqlite3 *db,
                                                      std::string seanerbus);
