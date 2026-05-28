// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "lib/formula.hpp"
#include "lib/util.hpp"
#include "tinyexpr.h"
#include <cerrno>
#include <climits>
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

static formula priority_formula;
static std::map<int, double> scores;

static double min(double a, double b) { return a < b ? a : b; }
static double max(double a, double b) { return a > b ? a : b; }

////

struct eval_formula_res {
  bool ok;
  double result;
};
static eval_formula_res eval_formula(formula formula_obj,
                                     std::map<std::string, double> &vars_over) {

  std::map<std::string, double> vars_merged = vars_over;
  for (const auto &[name, value] : formula_obj.vars) {
    if (vars_merged.find(name) != vars_merged.end())
      continue;
    vars_merged[name] = value;
  }

  auto *te_vars =
      (te_variable *)malloc((vars_merged.size() + 2) * sizeof(te_variable));
  int i = 0;

  for (const auto &[name, value] : vars_merged) {
    te_vars[i++] = {name.c_str(), &value};
  }
  te_vars[i++] = {"min", (void *)min, TE_FUNCTION2};
  te_vars[i++] = {"max", (void *)max, TE_FUNCTION2};

  int err = 0;
  te_expr *compiled = te_compile(formula_obj.expr.c_str(), te_vars, i, &err);
  if (!compiled)
    return {false, 0};
  auto result = te_eval(compiled);
  te_free(compiled);
  return {true, result};
}

static bool validate_formula(formula formula_obj) {
  std::map<std::string, double> vars;
  return eval_formula(formula_obj, vars).ok;
}

static void init_formula(flux_t *h) {
  flux_future_t *f;
  if (!(f = flux_kvs_lookup(h, NULL, 0, "priorisea.formula.priority"))) {
    return;
  }
  const char *formula_str;
  if (flux_kvs_lookup_get(f, &formula_str) < 0) {
    flux_future_destroy(f);
    return;
  }
  auto from_res = formula_from_str(formula_str);
  if (!from_res.ok || !validate_formula(from_res.formula_obj)) {
    flux_future_destroy(f);
    return;
  }
  priority_formula = from_res.formula_obj;
  flux_future_destroy(f);
}

////

static void get_formula_cb(flux_t *h, flux_msg_handler_t *mh,
                           const flux_msg_t *msg, void *arg) {
  auto formula_str = formula_to_str(priority_formula);
  if (flux_respond_pack(h, msg, "{s:s}", "data", formula_str.c_str()) < 0) {
    auto errmsg = fmt::format("respond pack failed: {}", strerror(errno));
    flux_log_error(h, "%s", errmsg.c_str());
  }
}

static void set_formula_cb(flux_t *h, flux_msg_handler_t *mh,
                           const flux_msg_t *msg, void *arg) {
  const char *formula_str = nullptr;
  if (flux_request_unpack(msg, NULL, "{s:s}", "data", &formula_str) < 0) {
    auto errmsg = fmt::format("payload unpack failed: {}", strerror(errno));
    flux_respond_error(h, msg, errno, errmsg.c_str());
    return;
  }

  if (!formula_str) {
    flux_respond_error(h, msg, EINVAL, "data is null");
    return;
  }

  auto from_res = formula_from_str(formula_str);
  if (!from_res.ok) {
    auto errmsg = fmt::format("invalid formula: {}", from_res.message);
    flux_respond_error(h, msg, EINVAL, errmsg.c_str());
    return;
  }

  if (!validate_formula(from_res.formula_obj)) {
    auto errmsg = fmt::format("invalid formula: {}", from_res.formula_obj.expr);
    flux_respond_error(h, msg, EINVAL, errmsg.c_str());
    return;
  }

  priority_formula = from_res.formula_obj;

  flux_respond(h, msg, NULL);
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

  auto *res = json_object();
  size_t num_data = json_array_size(arr);

  for (size_t i = 0; i < num_data; i++) {
    auto *elem = json_array_get(arr, i);

    if (!json_is_integer(elem)) {
      auto errmsg = fmt::format("element {} is not an integer", i);
      flux_respond_error(h, msg, EINVAL, errmsg.c_str());
      json_decref(res);
      return;
    }

    int user_id = json_integer_value(elem);
    auto it = scores.find(user_id);

    if (it == scores.end()) {
      auto errmsg = fmt::format("user_id {} not found", user_id);
      flux_respond_error(h, msg, EINVAL, errmsg.c_str());
      json_decref(res);
      return;
    }

    std::string user_id_key = std::to_string(user_id);
    json_object_set_new(res, user_id_key.c_str(), json_real(it->second));
  }

  if (flux_respond_pack(h, msg, "{s:o}", "data", res) < 0) {
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

  const char *user_id_key = nullptr;
  json_t *score_j = nullptr;

  json_object_foreach(obj, user_id_key, score_j) {
    int user_id = 0;

    if (!parse_int(user_id_key, user_id)) {
      auto errmsg = fmt::format("invalid user_id key '{}'", user_id_key);
      flux_respond_error(h, msg, EINVAL, errmsg.c_str());
      return;
    }

    if (!json_is_real(score_j)) {
      auto errmsg = fmt::format("score for user_id {} is not an real", user_id);
      flux_respond_error(h, msg, EINVAL, errmsg.c_str());
      return;
    }

    double score_value = json_real_value(score_j);
    scores[user_id] = score_value;
  }

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
    vars_over["priority"] = static_cast<double>(priority);
  }

  auto scores_it = scores.find(userid);
  if (scores_it != scores.end()) {
    vars_over["score"] = static_cast<double>(scores_it->second);
  }

  auto eval_res = eval_formula(priority_formula, vars_over);
  if (!eval_res.ok) {
    flux_log_error(h, "formula evaluation failed");
    return flux_jobtap_priority_unavail(p, args);
  }

  auto result = eval_res.result;
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

////

static const struct flux_plugin_handler tab[] = {
    {"job.state.priority", priority_cb, NULL},
    {"job.priority.get", priority_cb, NULL},
    {0},
};

extern "C" int flux_plugin_init(flux_plugin_t *p) {
  flux_t *h = flux_jobtap_get_flux(p);

  init_formula(h);

  if (flux_plugin_register(p, "sea_priority", tab) < 0 ||
      flux_jobtap_service_register(p, "get_formula", get_formula_cb, p) < 0 ||
      flux_jobtap_service_register(p, "set_formula", set_formula_cb, p) < 0 ||
      flux_jobtap_service_register(p, "get_score", get_score_cb, p) < 0 ||
      flux_jobtap_service_register(p, "set_score", set_score_cb, p) < 0)
    return -1;

  return 0;
}
