// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/daemon/endpoint/flux/callback/decay.hpp"
#include "src/database/decay.hpp"
#include "src/database/sqlite/decay.hpp"

#include "src/lib/decay.hpp"
#include "src/lib/error.hpp"

void ps_daemon_endpoint_flux_decay_get(flux_t *h, const flux_msg_t *msg,
                                       sqlite3 *db) {
  auto decay_res = ps_db_sqlite_decay_get(db);
  if (!ps_is_ok(decay_res)) {
    flux_respond_error(h, msg, 0, ps_error_message(decay_res).c_str());
    return;
  }
  auto decay = ps_unwrap(decay_res);
  flux_respond_pack(h, msg, "{s:o}", "data", ps_decay_to_json(decay));
}

void ps_daemon_endpoint_flux_decay_update(flux_t *h, const flux_msg_t *msg,
                                          sqlite3 *db) {
  json_t *decay_json;
  if (flux_request_unpack(msg, NULL, "{s:o}", "data", &decay_json) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
    return;
  }
  auto decay_res = ps_decay_from_json(decay_json);
  if (!ps_is_ok(decay_res)) {
    flux_respond_error(h, msg, 0, ps_error_message(decay_res).c_str());
    return;
  }
  auto decay = ps_unwrap(decay_res);
  auto update_res = ps_db_sqlite_decay_update(db, decay);
  if (!ps_is_ok(update_res)) {
    flux_respond_error(h, msg, 0, ps_error_message(update_res).c_str());
    return;
  }
  flux_respond(h, msg, NULL);
}

void ps_daemon_endpoint_flux_decay_reset(flux_t *h, const flux_msg_t *msg,
                                         sqlite3 *db) {
  auto reset_res = ps_db_decay_reset(db);
  if (!ps_is_ok(reset_res)) {
    flux_respond_error(h, msg, 0, ps_error_message(reset_res).c_str());
    return;
  }
  flux_respond(h, msg, NULL);
}
