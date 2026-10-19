// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/daemon/endpoint/flux/callback/info.hpp"
#include <src/database/sqlite/info.hpp>
#include <src/database/sqlite/sqlite.hpp>
#include <src/lib/error.hpp>

int ps_daemon_endpoint_flux_info_get(
    const ps_daemon_endpoint_flux_callback_args_t &args) {

  auto version_res = ps_db_sqlite_get_user_version(args.db);
  if (!ps_is_ok(version_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(version_res).c_str());
  }
  auto version = ps_unwrap(version_res);
  auto t_inactive_res = ps_db_sqlite_t_inactive_get(args.db);
  if (!ps_is_ok(t_inactive_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(t_inactive_res).c_str());
  }
  auto t_inactive = ps_unwrap(t_inactive_res);
  return flux_respond_pack(args.h, args.msg, "{s:{s:i s:f}}", "data", "version",
                           version, "t_inactive", t_inactive);
}
