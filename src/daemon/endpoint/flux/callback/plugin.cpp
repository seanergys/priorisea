// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/daemon/endpoint/flux/callback/plugin.hpp"
#include "src/daemon/endpoint/error.hpp"
#include "src/database/sqlite/formula.hpp"
#include "src/database/sqlite/user.hpp"
#include "src/lib/error.hpp"
#include "src/lib/formula.hpp"
#include <flux/core/response.h>
#include <flux/core/rpc.h>
#include <jansson.h>
#include <string>
#include <sys/types.h>

void ps_daemon_endpoint_flux_plugin_sync(flux_t *h, const flux_msg_t *msg,
                                         sqlite3 *db) {
  auto formula_res = ps_db_sqlite_formula_get(db, "job_priority");
  if (!ps_is_ok(formula_res)) {
    flux_respond_error(h, msg, 0, ps_error_message(formula_res).c_str());
    return;
  }
  auto formula_json = ps_formula_to_json(ps_unwrap(formula_res));

  auto users_res = ps_db_sqlite_users_list(db);
  if (!ps_is_ok(users_res)) {
    flux_respond_error(h, msg, 0, ps_error_message(users_res).c_str());
    return;
  }
  auto users = ps_unwrap(users_res);

  auto scores_json = json_object();
  for (const auto &user : users) {
    auto userid_str = std::to_string(user.userid);
    json_object_set(scores_json, userid_str.c_str(), json_real(user.score));
  }

  auto f = flux_rpc_pack(h, "job-manager.priorisea.plugin.sync",
                         FLUX_NODEID_ANY, 0, "{s:{s:o s:o}}", "data", "formula",
                         formula_json, "scores", scores_json);

  if (!f) {
    auto err = ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
    flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
    return;
  }

  if (flux_rpc_get(f, NULL) < 0) {
    auto err_str = std::string(flux_future_error_string(f));
    auto err = ps_plugin_err_t{.err = err_str};
    flux_future_destroy(f);
    flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
    return;
  }
  flux_future_destroy(f);
  flux_respond(h, msg, NULL);
}

void ps_daemon_endpoint_flux_plugin_debug(flux_t *h, const flux_msg_t *msg,
                                          sqlite3 *db) {
  auto f = flux_rpc(h, "job-manager.priorisea.plugin.debug", NULL,
                    FLUX_NODEID_ANY, 0);

  if (!f) {
    auto err = ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
    flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
    return;
  }

  const char *res;
  if (flux_rpc_get(f, &res) < 0) {
    auto err_str = std::string(flux_future_error_string(f));
    auto err = ps_plugin_err_t{.err = err_str};
    flux_future_destroy(f);
    flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
    return;
  }
  auto res_str = std::string(res);
  flux_future_destroy(f);
  flux_respond(h, msg, res_str.c_str());
}
