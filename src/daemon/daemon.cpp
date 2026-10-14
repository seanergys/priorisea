// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/daemon/daemon.hpp"
#include "src/daemon/endpoint/flux/endpoint.hpp"
#include "src/database/sqlite/sqlite.hpp"
#include "src/lib/error.hpp"
#include <csignal>
#include <cstring>
#include <flux/core.h>
#include <syslog.h>

void ps_daemon_run(const ps_daemon_config_t &config) {
  auto db_res = ps_db_sqlite_open(config.database_path);
  if (!ps_is_ok(db_res)) {
    ps_exit_error(db_res);
  }
  auto db = ps_unwrap(db_res);
  auto verify_res = ps_db_sqlite_verify(db);
  if (!ps_is_ok(verify_res)) {
    ps_exit_error(verify_res);
  }
  auto run_res = ps_daemon_endpoint_flux_run(db, config.seanerbus_path);
  if (!ps_is_ok(run_res)) {
    ps_exit_error(run_res);
  }
}
