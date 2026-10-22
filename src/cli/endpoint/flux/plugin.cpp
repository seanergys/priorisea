#include "src/cli/endpoint/flux/plugin.hpp"
#include "src/cli/endpoint/util.hpp"
#include "src/lib/error.hpp"
#include "src/lib/formula.hpp"
#include "src/lib/util.hpp"
#include <flux/core/rpc.h>
#include <jansson.h>
#include <string>
#include <sys/types.h>

ps_result_t<ps_success_t> ps_cli_endpoint_flux_plugin_sync() {
  auto h = flux_open(NULL, 0);
  if (!h) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }
  auto f =
      flux_rpc(h, "priorisea.daemon.plugin.sync", NULL, FLUX_NODEID_ANY, 0);
  if (!f) {
    flux_close(h);
    return ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
  }

  if (flux_rpc_get(f, NULL) < 0) {
    auto err = std::string(flux_future_error_string(f));
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(ps_daemon_err_t{.err = err});
  }
  flux_future_destroy(f);
  flux_close(h);
  return ps_success_t{};
}

ps_result_t<ps_cli_endpoint_plugin_debug_t>
ps_cli_endpoint_flux_plugin_debug() {
  auto h = flux_open(NULL, 0);
  if (!h) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }
  auto f =
      flux_rpc(h, "priorisea.daemon.plugin.debug", NULL, FLUX_NODEID_ANY, 0);
  if (!f) {
    flux_close(h);
    return ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
  }

  json_t *formula_json;
  json_t *scores_json;
  if (flux_rpc_get_unpack(f, "{s:{s:o s:o}}", "data", "formula", &formula_json,
                          "scores", &scores_json) < 0) {
    auto err = ps_cli_endpoint_unpack_err(f);
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(err);
  }

  auto formula_res = ps_formula_from_json(formula_json);
  if (!ps_is_ok(formula_res)) {
    flux_future_destroy(f);
    flux_close(h);
    return ps_forward_error<ps_cli_endpoint_plugin_debug_t>(formula_res);
  }
  auto formula = ps_unwrap(formula_res);

  if (!json_is_object(scores_json)) {
    return ps_error(ps_unexpected_err_t{
        .msg =
            "priorisea.daemon.plugin.debug.scores did not return an object"});
  }

  std::map<uid_t, double> scores;
  const char *key;
  json_t *value;
  json_object_foreach(scores_json, key, value) {
    uid_t uid;
    if (!parse_uid(key, uid)) {
      auto key_str = std::string(key);
      auto err = "priorisea.daemon.plugin.debug.scores." + key_str +
                 "is not convertible to uid_t";
      return ps_error(ps_unexpected_err_t{.msg = err});
    }
    if (!json_is_real(value)) {
      auto key_str = std::string(key);
      auto err = "priorisea.daemon.plugin.debug.scores." + key_str +
                 " value is not convertible to a double";
      return ps_error(ps_unexpected_err_t{.msg = err});
    }
    scores[uid] = json_real_value(value);
  }

  flux_future_destroy(f);
  flux_close(h);
  return ps_cli_endpoint_plugin_debug_t{.formula = formula, .scores = scores};
}
