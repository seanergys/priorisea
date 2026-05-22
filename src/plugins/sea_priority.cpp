// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "tinyexpr.h"
#include "util.hpp"
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <flux/core.h>
#include <flux/jobtap.h>
#include <fmt/format.h>
#include <jansson.h>
#include <map>
#include <string.h>
#include <string>

static const int DEFAULT_SCORE = 0;
static const int DEFAULT_PRIORITY = 0;
static std::string formula;
static std::map<int, int> scores;

static double min(double a, double b) { return a < b ? a : b; }
static double max(double a, double b) { return a > b ? a : b; }

////

struct eval_formula_res {
  bool ok;
  double result;
};
static eval_formula_res eval_formula(const std::string &expr, double priority,
                                     double score) {
  te_variable vars[] = {{"priority", &priority},
                        {"score", &score},
                        {"min", (void *)min, TE_FUNCTION2},
                        {"max", (void *)max, TE_FUNCTION2}};
  int err = 0;
  te_expr *compiled = te_compile(expr.c_str(), vars, 3, &err);
  if (!compiled)
    return {false, 0};
  auto result = te_eval(compiled);
  te_free(compiled);
  return {true, result};
}

static bool validate_formula(const std::string &expr) {
  return eval_formula(expr, 0, 0).ok;
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
  if (!validate_formula(formula_str)) {
    flux_future_destroy(f);
    return;
  }
  formula = formula_str;
  flux_future_destroy(f);
}

////

static void get_formula_cb(flux_t *h, flux_msg_handler_t *mh,
                           const flux_msg_t *msg, void *arg) {
  if (flux_respond_pack(h, msg, "{s:s}", "data", formula.c_str()) < 0) {
    auto errmsg = fmt::format("respond pack failed: {}", strerror(errno));
    flux_log_error(h, "%s", errmsg.c_str());
  }
}

static void set_formula_cb(flux_t *h, flux_msg_handler_t *mh,
                           const flux_msg_t *msg, void *arg) {
  const char *formula_value = nullptr;

  if (flux_request_unpack(msg, NULL, "{s:s}", "data", &formula_value) < 0) {
    auto errmsg = fmt::format("payload unpack failed: {}", strerror(errno));
    flux_respond_error(h, msg, errno, errmsg.c_str());
    return;
  }

  if (!formula_value) {
    flux_respond_error(h, msg, EINVAL, "data is null");
    return;
  }

  if (!validate_formula(formula_value)) {
    auto errmsg = fmt::format("invalid formula: {}", formula_value);
    flux_respond_error(h, msg, EINVAL, errmsg.c_str());
    return;
  }

  formula = formula_value;

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

    json_int_t user_id_j = json_integer_value(elem);

    if (user_id_j < INT_MIN || user_id_j > INT_MAX) {
      auto errmsg = fmt::format("user_id at element {} is out of int range", i);
      flux_respond_error(h, msg, EINVAL, errmsg.c_str());
      json_decref(res);
      return;
    }

    int user_id = static_cast<int>(user_id_j);
    auto it = scores.find(user_id);

    if (it == scores.end()) {
      auto errmsg = fmt::format("user_id {} not found", user_id);
      flux_respond_error(h, msg, EINVAL, errmsg.c_str());
      json_decref(res);
      return;
    }

    std::string user_id_key = std::to_string(user_id);
    json_object_set_new(res, user_id_key.c_str(), json_integer(it->second));
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

    if (!json_is_integer(score_j)) {
      auto errmsg =
          fmt::format("score for user_id {} is not an integer", user_id);
      flux_respond_error(h, msg, EINVAL, errmsg.c_str());
      return;
    }

    json_int_t score_value = json_integer_value(score_j);

    if (score_value < INT_MIN || score_value > INT_MAX) {
      auto errmsg =
          fmt::format("score for user_id {} is out of int range", user_id);
      flux_respond_error(h, msg, EINVAL, errmsg.c_str());
      return;
    }

    scores[user_id] = static_cast<int>(score_value);
  }

  flux_respond(h, msg, NULL);
}

static int priority_cb(flux_plugin_t *p, const char *topic,
                       flux_plugin_arg_t *args, void *data) {
  flux_t *h = flux_jobtap_get_flux(p);

  int userid;
  if (flux_plugin_arg_unpack(args, FLUX_PLUGIN_ARG_IN, "{s:i}", "userid",
                             &userid) < 0) {
    flux_log_error(h, "%s", flux_plugin_arg_strerror(args));
    return flux_jobtap_priority_unavail(p, args);
  }

  int64_t priority;
  if (flux_plugin_arg_unpack(args, FLUX_PLUGIN_ARG_OUT, "{s:I}", "priority",
                             &priority) < 0) {
    priority = DEFAULT_PRIORITY;
  }
  double priority_d = static_cast<double>(priority);

  double score = DEFAULT_SCORE;
  auto it = scores.find(userid);
  if (it != scores.end()) {
    score = it->second;
  }

  if (formula.empty()) {
    flux_log_error(h, "formula not set");
    return flux_jobtap_priority_unavail(p, args);
  }
  auto eval_res = eval_formula(formula, priority, score);
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
