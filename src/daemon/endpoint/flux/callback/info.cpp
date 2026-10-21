// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/daemon/endpoint/flux/callback/info.hpp"
#include "src/lib/info.hpp"
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
  auto info_res = ps_db_sqlite_info_get(args.db);
  if (!ps_is_ok(info_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(info_res).c_str());
  }
  auto info = ps_unwrap(info_res);
  auto *info_json = ps_db_info_to_json(info);
  return flux_respond_pack(args.h, args.msg, "{s:{s:i s:o}}", "data", "version",
                           version, "info", info_json);
}
