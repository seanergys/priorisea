// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/daemon/endpoint/flux/callback/decay.hpp"
#include "src/database/decay.hpp"
#include "src/database/sqlite/decay.hpp"

#include "src/lib/decay.hpp"
#include "src/lib/error.hpp"

int ps_daemon_endpoint_flux_decay_get(
    const ps_daemon_endpoint_flux_callback_args_t &args) {
  auto decay_res = ps_db_sqlite_decay_get(args.db);
  if (!ps_is_ok(decay_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(decay_res).c_str());
  }
  auto decay = ps_unwrap(decay_res);
  return flux_respond_pack(args.h, args.msg, "{s:o}", "data",
                           ps_decay_to_json(decay));
}

int ps_daemon_endpoint_flux_decay_update(
    const ps_daemon_endpoint_flux_callback_args_t &args) {
  json_t *decay_json;
  if (flux_request_unpack(args.msg, NULL, "{s:o}", "data", &decay_json) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }
  auto decay_res = ps_decay_from_json(decay_json);
  if (!ps_is_ok(decay_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(decay_res).c_str());
  }
  auto decay = ps_unwrap(decay_res);
  auto update_res = ps_db_sqlite_decay_update(args.db, decay);
  if (!ps_is_ok(update_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(update_res).c_str());
  }
  return flux_respond(args.h, args.msg, NULL);
}

int ps_daemon_endpoint_flux_decay_reset(
    const ps_daemon_endpoint_flux_callback_args_t &args) {
  auto reset_res = ps_db_decay_reset(args.db);
  if (!ps_is_ok(reset_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(reset_res).c_str());
  }
  return flux_respond(args.h, args.msg, NULL);
}
