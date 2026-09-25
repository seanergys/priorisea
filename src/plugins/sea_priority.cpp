// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "lib/config.hpp"
#include "lib/error.hpp"
#include "lib/flux.hpp"
#include "lib/formula.hpp"
#include "lib/score.hpp"
#include "lib/types.hpp"
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <flux/core.h>
#include <flux/core/flog.h>
#include <flux/jobtap.h>
#include <fmt/format.h>
#include <jansson.h>
#include <map>
#include <string.h>
#include <string>

static ps_formula_t priority_formula;
static ps_score_map_t scores;
static ps_seanerbus_t seanerbus_cfg;

static void init_formula() {
  auto get_res = ps_db_formula_get("job_priority");
  if (ps_is_ok(get_res)) {
    priority_formula = ps_unwrap(get_res);
  }
}

static void init_scores() {
  auto list_res = ps_db_users_list();
  if (!ps_is_ok(list_res))
    return;
  for (const auto &user_res : ps_unwrap(list_res)) {
    if (ps_is_ok(user_res.result)) {
      auto urr = user_res.result;
      scores[user_res.userid] = ps_unwrap(urr).score;
    }
  }
}

static int init_config(flux_t *h) {
  std::string config_path = ps_config_default_path();
  auto config_res = ps_config_load(config_path);
  if (!ps_is_ok(config_res)) {
    flux_log_error(h, "failed to read config: %s", strerror(errno));
    return -1;
  }
  auto config = ps_unwrap(config_res);
  seanerbus_cfg = config.seanerbus;
  return 0;
}

////

static void get_formula_cb(flux_t *h, flux_msg_handler_t *mh,
                           const flux_msg_t *msg, void *arg) {
  auto *json_formula = ps_formula_to_json(priority_formula);
  if (flux_respond_pack(h, msg, "{s:o}", "data", json_formula) < 0) {

    auto errmsg = fmt::format("respond pack failed: {}", strerror(errno));
    flux_log_error(h, "%s", errmsg.c_str());
  }
}

static void set_formula_cb(flux_t *h, flux_msg_handler_t *mh,
                           const flux_msg_t *msg, void *arg) {
  json_t *json_formula = NULL;

  if (flux_request_unpack(msg, NULL, "{s:o}", "data", &json_formula) < 0) {
    auto errmsg = fmt::format("payload unpack failed: {}", strerror(errno));
    flux_respond_error(h, msg, errno, errmsg.c_str());
    return;
  }

  auto formula_res = ps_formula_from_json(json_formula);
  if (!ps_is_ok(formula_res)) {
    auto errmsg = ps_error_message(formula_res);
    flux_respond_error(h, msg, EINVAL, errmsg.c_str());
    return;
  }
  auto formula = ps_unwrap(formula_res);

  if (!ps_validate_formula(formula)) {
    auto errmsg = fmt::format("invalid formula: {}", formula.expr);
    flux_respond_error(h, msg, EINVAL, errmsg.c_str());
    return;
  }

  priority_formula = formula;

  if (flux_respond(h, msg, NULL) < 0)
    flux_log_error(h, "flux_respond");
  return;
}

static void get_score_cb(flux_t *h, flux_msg_handler_t *mh,
                         const flux_msg_t *msg, void *arg) {
  json_t *arr = NULL;

  if (flux_request_unpack(msg, NULL, "{s:o}", "data", &arr) < 0) {
    auto errmsg = fmt::format("payload unpack failed: {}", strerror(errno));
    flux_respond_error(h, msg, errno, errmsg.c_str());
    return;
  }

  if (!json_is_array(arr)) {
    flux_respond_error(h, msg, EINVAL, "data is not an array");
    return;
  }

  // Build a temporary map of requested userids to look up scores
  ps_score_map_t result_map;
  size_t num_data = json_array_size(arr);

  for (size_t i = 0; i < num_data; i++) {
    auto *elem = json_array_get(arr, i);

    if (!json_is_integer(elem)) {
      auto errmsg = fmt::format("element {} is not an integer", i);
      flux_respond_error(h, msg, EINVAL, errmsg.c_str());
      return;
    }

    int user_id = json_integer_value(elem);
    auto it = scores.find(user_id);

    if (it == scores.end()) {
      auto errmsg = fmt::format("user_id {} not found", user_id);
      flux_respond_error(h, msg, EINVAL, errmsg.c_str());
      return;
    }

    result_map[user_id] = it->second;
  }

  auto *res = score_map_to_json(result_map);
  if (flux_respond_pack(h, msg, "{s:o}", "data", res) < 0) {
    json_decref(res);
    auto errmsg = fmt::format("respond pack failed: {}", strerror(errno));
    flux_log_error(h, "%s", errmsg.c_str());
  }
}

static void set_score_cb(flux_t *h, flux_msg_handler_t *mh,
                         const flux_msg_t *msg, void *arg) {
  json_t *obj = NULL;

  if (flux_request_unpack(msg, NULL, "{s:o}", "data", &obj) < 0) {
    auto errmsg = fmt::format("payload unpack failed: {}", strerror(errno));
    flux_respond_error(h, msg, errno, errmsg.c_str());
    return;
  }

  if (!json_is_object(obj)) {
    flux_respond_error(h, msg, EINVAL, "data is not an object");
    return;
  }

  auto new_scores_res = score_map_from_json(obj);
  if (!ps_is_ok(new_scores_res)) {
    auto errmsg = ps_error_message(new_scores_res);
    flux_respond_error(h, msg, EINVAL, errmsg.c_str());
    return;
  }
  auto new_scores = ps_unwrap(new_scores_res);
  scores.insert(new_scores.begin(), new_scores.end());

  flux_respond(h, msg, NULL);
}

static void get_seanerbus_cb(flux_t *h, flux_msg_handler_t *mh,
                             const flux_msg_t *msg, void *arg) {
  auto *json_seanerbus = ps_seanerbus_to_json(seanerbus_cfg);
  if (flux_respond_pack(h, msg, "{s:o}", "data", json_seanerbus) < 0) {
    json_decref(json_seanerbus);
    auto errmsg = fmt::format("respond pack failed: {}", strerror(errno));
    flux_log_error(h, "%s", errmsg.c_str());
  }
}

static void set_seanerbus_cb(flux_t *h, flux_msg_handler_t *mh,
                             const flux_msg_t *msg, void *arg) {
  json_t *json_seanerbus = NULL;
  if (flux_request_unpack(msg, NULL, "{s:o}", "data", &json_seanerbus) < 0) {
    auto errmsg = fmt::format("payload unpack failed: {}", strerror(errno));
    flux_respond_error(h, msg, errno, errmsg.c_str());
    return;
  }

  auto seanerbus_res = ps_seanerbus_from_json(json_seanerbus);
  if (!ps_is_ok(seanerbus_res)) {
    auto errmsg = ps_error_message(seanerbus_res);
    flux_respond_error(h, msg, EINVAL, errmsg.c_str());
    return;
  }
  seanerbus_cfg = ps_unwrap(seanerbus_res);

  flux_respond(h, msg, NULL);
}

////

static int priority_cb(flux_plugin_t *p, const char *topic,
                       flux_plugin_arg_t *args, void *data) {
  flux_t *h = flux_jobtap_get_flux(p);

  int userid;
  if (flux_plugin_arg_unpack(args, FLUX_PLUGIN_ARG_IN, "{s:i}", "userid",
                             &userid) < 0) {
    flux_log_error(h, "%s", flux_plugin_arg_strerror(args));
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

  auto eval_res = ps_eval_formula(priority_formula, vars_over);
  if (!ps_is_ok(eval_res)) {
    auto errmsg = ps_error_message(eval_res);
    flux_log_error(h, "formula evaluation failed: %s", errmsg.c_str());
    return flux_jobtap_priority_unavail(p, args);
  }

  auto result = ps_unwrap(eval_res);
  if (!std::isfinite(result)) {
    flux_log_error(h, "formula evaluated to NaN or infinity");
    return flux_jobtap_priority_unavail(p, args);
  }

  if (result < static_cast<double>(std::numeric_limits<int64_t>::min()) ||
      result > static_cast<double>(std::numeric_limits<int64_t>::max())) {
    flux_log_error(h, "formula result out of int64_t range");
    return flux_jobtap_priority_unavail(p, args);
  }
  priority = static_cast<int64_t>(result);

  if (flux_plugin_arg_pack(args, FLUX_PLUGIN_ARG_OUT, "{s:I}", "priority",
                           priority) < 0) {
    flux_log_error(h, "%s", flux_plugin_arg_strerror(args));
    return flux_jobtap_priority_unavail(p, args);
  }

  return 0;
}

static int conf_update_cb(flux_plugin_t *p, const char *topic,
                          flux_plugin_arg_t *args, void *arg) {
  json_t *conf;
  if (flux_plugin_arg_unpack(args, FLUX_PLUGIN_ARG_IN, "{s:o}", "conf", &conf) <
      0) {
    auto errmsg = fmt::format("error unpacking conf.update arguments: {}",
                              strerror(errno));
    return flux_jobtap_error(p, args, "%s", errmsg.c_str());
  }

  json_t *seanerbus_conf = NULL;
  if (json_unpack(conf, "{s{s?o}}", "priorisea", "seanerbus", &seanerbus_conf) <
      0)
    return 0;
  if (seanerbus_conf) {
    auto seanerbus_res = ps_seanerbus_from_json(seanerbus_conf);
    if (ps_is_ok(seanerbus_res)) {
      seanerbus_cfg = ps_unwrap(seanerbus_res);
    }
  }
  return 0;
}

////

static const struct flux_plugin_handler tab[] = {
    {"job.state.priority", priority_cb, NULL},
    {"job.priority.get", priority_cb, NULL},
    {"conf.update", conf_update_cb, NULL},
    {0},
};

extern "C" int flux_plugin_init(flux_plugin_t *p) {
  flux_t *h = flux_jobtap_get_flux(p);
  if (init_config(h) < 0) {
    return -1;
  }
  init_formula();
  init_scores();
  if (flux_plugin_register(p, "sea_priority", tab) < 0 ||
      flux_jobtap_service_register(p, "get_formula", get_formula_cb, p) < 0 ||
      flux_jobtap_service_register(p, "set_formula", set_formula_cb, p) < 0 ||
      flux_jobtap_service_register(p, "get_score", get_score_cb, p) < 0 ||
      flux_jobtap_service_register(p, "set_score", set_score_cb, p) < 0 ||
      flux_jobtap_service_register(p, "get_seanerbus", get_seanerbus_cb, p) <
          0 ||
      flux_jobtap_service_register(p, "set_seanerbus", set_seanerbus_cb, p) < 0)
    return -1;

  return 0;
}
