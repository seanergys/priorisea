#include "src/lib/error.hpp"
#include "src/lib/formula.hpp"
#include "src/lib/types.hpp"
#include "src/lib/util.hpp"
#include <cmath>
#include <cstring>
#include <flux/core.h>
#include <flux/core/request.h>
#include <flux/core/response.h>
#include <flux/jobtap.h>
#include <jansson.h>
#include <string>
#include <sys/syslog.h>

static ps_formula_t formula;
static std::map<uid_t, double> scores;

static void sync_cb(flux_t *h, flux_msg_handler_t *mh, const flux_msg_t *msg,
                    void *arg) {
  json_t *formula_json;
  json_t *scores_json;
  if (flux_request_unpack(msg, NULL, "{s:{s:o s:o}}", "data", "formula",
                          &formula_json, "scores", &scores_json) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
    return;
  }
  auto formula_res = ps_formula_from_json(formula_json);
  if (!ps_is_ok(formula_res)) {
    flux_respond_error(h, msg, 0, ps_error_message(formula_res).c_str());
    return;
  }
  if (!json_is_object(scores_json)) {
    auto err =
        ps_field_type_mismatch_t{.field = "data.scores", .expected = "object"};
    flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
    return;
  }
  const char *key;
  json_t *value;
  std::map<uid_t, double> _scores;
  json_object_foreach(scores_json, key, value) {
    uid_t user;
    if (!parse_uid(key, user)) {
      auto err = ps_field_type_mismatch_t{
          .field = "data.scores." + std::string(key), .expected = "uid"};
      flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
      return;
    }
    if (!json_is_real(value)) {
      auto err = ps_field_type_mismatch_t{.field = "data.scores[" +
                                                   std::string(key) + "]",
                                          .expected = "double"};
      flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
      return;
    }
    _scores[user] = json_real_value(value);
  }

  formula = ps_unwrap(formula_res);
  scores = _scores;
  flux_respond(h, msg, NULL);
}

static void debug_cb(flux_t *h, flux_msg_handler_t *mh, const flux_msg_t *msg,
                     void *arg) {
  auto formula_json = ps_formula_to_json(formula);
  auto scores_json = json_object();
  for (const auto &[user, score] : scores) {
    json_object_set(scores_json, std::to_string(user).c_str(),
                    json_real(score));
  }
  if (flux_respond_pack(h, msg, "{s:{s:o s:o}}", "data", "formula",
                        formula_json, "scores", scores_json) < 0) {
    flux_log_error(h, "%s: %s", __FUNCTION__, strerror(errno));
  }
}

static int priority_cb(flux_plugin_t *p, const char *topic,
                       flux_plugin_arg_t *args, void *data) {
  flux_t *h = flux_jobtap_get_flux(p);

  int userid;
  if (flux_plugin_arg_unpack(args, FLUX_PLUGIN_ARG_IN, "{s:i}", "userid",
                             &userid) < 0) {
    flux_log_error(h, "%s: %s", __FUNCTION__, flux_plugin_arg_strerror(args));
    return flux_jobtap_priority_unavail(p, args);
  }

  std::map<std::string, double> vars_over;

  int64_t priority;
  if (flux_plugin_arg_unpack(args, FLUX_PLUGIN_ARG_OUT, "{s:I}", "priority",
                             &priority) >= 0) {
    vars_over["var:prev_priority"] = static_cast<double>(priority);
  }

  auto scores_it = scores.find(userid);
  if (scores_it != scores.end()) {
    vars_over["var:user_score"] = static_cast<double>(scores_it->second);
  }

  auto eval_res = ps_eval_formula(formula, vars_over);
  if (!ps_is_ok(eval_res)) {
    flux_log_error(h, "%s: %s", __FUNCTION__,
                   ps_error_message(eval_res).c_str());
    return flux_jobtap_priority_unavail(p, args);
  }

  auto result = ps_unwrap(eval_res);
  if (!std::isfinite(result)) {
    flux_log_error(h, "%s: formula evaluated to NaN or infinity", __FUNCTION__);
    return flux_jobtap_priority_unavail(p, args);
  }

  if (result < static_cast<double>(std::numeric_limits<int64_t>::min()) ||
      result > static_cast<double>(std::numeric_limits<int64_t>::max())) {
    flux_log_error(h, "%s: formula result out of int64_t range", __FUNCTION__);
    return flux_jobtap_priority_unavail(p, args);
  }
  priority = static_cast<int64_t>(result);

  if (flux_plugin_arg_pack(args, FLUX_PLUGIN_ARG_OUT, "{s:I}", "priority",
                           priority) < 0) {
    flux_log_error(h, "%s: %s", __FUNCTION__, flux_plugin_arg_strerror(args));
    return flux_jobtap_priority_unavail(p, args);
  }

  return 0;
}

static const struct flux_plugin_handler tab[] = {
    {"job.state.priority", priority_cb, NULL},
    {"job.priority.get", priority_cb, NULL},
    {0},
};

extern "C" int flux_plugin_init(flux_plugin_t *p) {
  flux_t *h = flux_jobtap_get_flux(p);
  if (flux_plugin_register(p, "priorisea.plugin", tab) < 0 ||
      flux_jobtap_service_register(p, "sync", sync_cb, p) < 0 ||
      flux_jobtap_service_register(p, "debug", debug_cb, p) < 0)
    return -1;

  return 0;
}
