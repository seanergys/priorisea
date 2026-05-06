#include "tinyexpr.h"
#include "util.hpp"
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <flux/core.h>
#include <flux/jobtap.h>
#include <fmt/core.h>
#include <jansson.h>
#include <map>
#include <string.h>
#include <string>

std::map<std::string, std::string> formulas;
std::map<int, int> scores;

static bool validate_priority(const std::string &expr) {
  double priority = 0;
  double score = 0;
  te_variable vars[] = {{"priority", &priority}, {"score", &score}};

  int err = 0;
  te_expr *compiled = te_compile(expr.c_str(), vars, 2, &err);
  if (!compiled)
    return false;

  te_free(compiled);
  return true;
}

static void get_formula_cb(flux_t *h, flux_msg_handler_t *mh,
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

    if (!json_is_string(elem)) {
      auto errmsg = fmt::format("element {} is not a string", i);
      flux_respond_error(h, msg, EINVAL, errmsg.c_str());
      json_decref(res);
      return;
    }

    const char *formula_name = json_string_value(elem);
    auto it = formulas.find(formula_name);

    if (it == formulas.end()) {
      auto errmsg = fmt::format("formula {} not found", formula_name);
      flux_respond_error(h, msg, EINVAL, errmsg.c_str());
      json_decref(res);
      return;
    }

    json_object_set_new(res, formula_name, json_string(it->second.c_str()));
  }

  if (flux_respond_pack(h, msg, "{s:o}", "data", res) < 0) {
    auto errmsg = fmt::format("respond pack failed: {}", strerror(errno));
    flux_log_error(h, "%s", errmsg.c_str());
  }
}

static void set_formula_cb(flux_t *h, flux_msg_handler_t *mh,
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

  const char *name = nullptr;
  json_t *formula_j = nullptr;

  json_object_foreach(obj, name, formula_j) {
    if (!json_is_string(formula_j)) {
      auto errmsg = fmt::format("formula '{}' is not a string", name);
      flux_respond_error(h, msg, EINVAL, errmsg.c_str());
      return;
    }

    const char *formula = json_string_value(formula_j);

    if (strcmp(name, "priority") == 0) {
      if (!validate_priority(formula)) {
        auto errmsg = fmt::format("invalid priority formula: {}", formula);
        flux_respond_error(h, msg, EINVAL, errmsg.c_str());
        return;
      }
    }

    formulas[name] = formula;
  }

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

static const struct flux_plugin_handler tab[] = {
    {0},
};

extern "C" int flux_plugin_init(flux_plugin_t *p) {
  if (flux_plugin_register(p, "sea_priority", tab) < 0 ||
      flux_jobtap_service_register(p, "get_formula", get_formula_cb, p) < 0 ||
      flux_jobtap_service_register(p, "set_formula", set_formula_cb, p) < 0 ||
      flux_jobtap_service_register(p, "get_score", get_score_cb, p) < 0 ||
      flux_jobtap_service_register(p, "set_score", set_score_cb, p) < 0)
    return -1;

  return 0;
}
