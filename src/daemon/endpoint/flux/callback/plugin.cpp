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

int ps_daemon_endpoint_flux_plugin_sync(
    const ps_daemon_endpoint_flux_callback_args_t &args) {
  auto formula_res = ps_db_sqlite_formula_get(args.db, "job_priority");
  if (!ps_is_ok(formula_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(formula_res).c_str());
  }
  auto formula_json = ps_formula_to_json(ps_unwrap(formula_res));

  auto users_res = ps_db_sqlite_users_list(args.db);
  if (!ps_is_ok(users_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(users_res).c_str());
  }
  auto users = ps_unwrap(users_res);

  auto scores_json = json_object();
  for (const auto &user : users) {
    auto userid_str = std::to_string(user.userid);
    json_object_set(scores_json, userid_str.c_str(), json_real(user.score));
  }

  auto f = flux_rpc_pack(args.h, "job-manager.priorisea.plugin.sync",
                         FLUX_NODEID_ANY, 0, "{s:{s:o s:o}}", "data", "formula",
                         formula_json, "scores", scores_json);

  if (!f) {
    auto err = ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }

  if (flux_rpc_get(f, NULL) < 0) {
    auto err_str = std::string(flux_future_error_string(f));
    auto err = ps_plugin_err_t{.err = err_str};
    flux_future_destroy(f);
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }
  flux_future_destroy(f);
  return flux_respond(args.h, args.msg, NULL);
}

int ps_daemon_endpoint_flux_plugin_debug(
    const ps_daemon_endpoint_flux_callback_args_t &args) {
  auto f = flux_rpc(args.h, "job-manager.priorisea.plugin.debug", NULL,
                    FLUX_NODEID_ANY, 0);

  if (!f) {
    auto err = ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }

  const char *res;
  if (flux_rpc_get(f, &res) < 0) {
    auto err_str = std::string(flux_future_error_string(f));
    auto err = ps_plugin_err_t{.err = err_str};
    flux_future_destroy(f);
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }
  auto res_str = std::string(res);
  flux_future_destroy(f);
  return flux_respond(args.h, args.msg, res_str.c_str());
}
