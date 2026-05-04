#include "exprtk.hpp"
#include <cerrno>
#include <errno.h>
#include <flux/core.h>
#include <flux/jobtap.h>
#include <fmt/core.h>
#include <jansson.h>
#include <map>
#include <string.h>
#include <string>

using namespace std;

map<string, string> formulas;
map<int, int> scores;

static bool validate_priority(const std::string &expr) {
  /*exprtk::symbol_table<double> symbol_table;
  exprtk::expression<double> expression;
  exprtk::parser<double> parser;
  double priority, score = 0;
  symbol_table.add_variable("priority", priority);
  symbol_table.add_variable("score", score);
  expression.register_symbol_table(symbol_table);
  return parser.compile(expr, expression);*/
  return true;
}

static void get_formula_cb(flux_t *h, flux_msg_handler_t *mh,
                           const flux_msg_t *msg, void *arg) {
  json_t *arr = NULL;
  if (flux_request_unpack(msg, NULL, "{s:o}", "data", &arr) < 0) {
    auto errmsg = fmt::format("payload unpack failed: {}", strerror(errno));
    // flux_log_error (h, "%s", errmsg);
    flux_respond_error(h, msg, errno, errmsg.c_str());
    return;
  }

  if (!json_is_array(arr)) {
    flux_respond_error(h, msg, EINVAL, "data is not an array");
    return;
  }

  auto *res = json_array();
  size_t num_data = json_array_size(arr);
  for (auto i = 0; i < num_data; i++) {
    auto *elem = json_array_get(arr, i);
    if (!json_is_string(elem)) {
      auto errmsg = fmt::format("element {} is not a string", i);
      flux_respond_error(h, msg, EINVAL, errmsg.c_str());
      return;
    }

    auto formula_name = json_string_value(elem);
    auto it = formulas.find(formula_name);
    if (it == formulas.end()) {
      auto errmsg = fmt::format("formula {} not found", formula_name);
      flux_respond_error(h, msg, EINVAL, errmsg.c_str());
      return;
    }

    auto formula_json = json_string(it->second.c_str());
    auto value = json_object();
    json_object_set_new(value, "name", elem);
    json_object_set_new(value, "formula", formula_json);
    json_array_append_new(res, value);
  }
  if (flux_respond_pack(h, msg, "{s:o}", "data", res) < 0) {
    auto errmsg = fmt::format("respond pack failed: {}", strerror(errno));
    flux_log_error(h, "%s", errmsg.c_str());
    // flux_respond_error(h, msg, errno, errmsg.c_str());
  }
}

static void set_formula_cb(flux_t *h, flux_msg_handler_t *mh,
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
  size_t num = json_array_size(arr);
  for (size_t i = 0; i < num; i++) {
    json_t *item = json_array_get(arr, i);

    if (!json_is_object(item)) {
      auto errmsg = fmt::format("element {} is not an object", i);
      flux_respond_error(h, msg, EINVAL, errmsg.c_str());
      return;
    }

    json_t *name_j = json_object_get(item, "name");
    json_t *formula_j = json_object_get(item, "formula");

    if (!json_is_string(name_j) || !json_is_string(formula_j)) {
      auto errmsg = fmt::format("element {} missing name/formula", i);
      flux_respond_error(h, msg, EINVAL, errmsg.c_str());
      return;
    }

    const char *name = json_string_value(name_j);
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
    // flux_log_error (h, "%s", errmsg);
    flux_respond_error(h, msg, errno, errmsg.c_str());
    return;
  }

  if (!json_is_array(arr)) {
    flux_respond_error(h, msg, EINVAL, "data is not an array");
    return;
  }

  auto *res = json_array();
  size_t num_data = json_array_size(arr);
  for (auto i = 0; i < num_data; i++) {
    auto *elem = json_array_get(arr, i);
    if (!json_is_integer(elem)) {
      auto errmsg = fmt::format("element {} is not an integer", i);
      flux_respond_error(h, msg, EINVAL, errmsg.c_str());
      return;
    }
    auto user_id = json_integer_value(elem);
    auto it = scores.find(user_id);
    if (it == scores.end()) {
      auto errmsg = fmt::format("user_id {} not found", user_id);
      flux_respond_error(h, msg, EINVAL, errmsg.c_str());
      return;
    }
    auto score_json = json_integer(it->second);
    auto value = json_object();
    json_object_set_new(value, "user_id", elem);
    json_object_set_new(value, "score", score_json);
    json_array_append_new(res, value);
  }
  if (flux_respond_pack(h, msg, "{s:o}", "data", res) < 0) {
    auto errmsg = fmt::format("respond pack failed: {}", strerror(errno));
    flux_log_error(h, "%s", errmsg.c_str());
    // flux_respond_error(h, msg, errno, errmsg.c_str());
  }
}

static void set_score_cb(flux_t *h, flux_msg_handler_t *mh,
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
  size_t num = json_array_size(arr);
  for (size_t i = 0; i < num; i++) {
    json_t *item = json_array_get(arr, i);

    if (!json_is_object(item)) {
      auto errmsg = fmt::format("element {} is not an object", i);
      flux_respond_error(h, msg, EINVAL, errmsg.c_str());
      return;
    }

    json_t *uid_j = json_object_get(item, "user_id");
    json_t *score_j = json_object_get(item, "score");

    if (!json_is_integer(uid_j) || !json_is_integer(score_j)) {
      auto errmsg = fmt::format("element {} missing user_id/score", i);
      flux_respond_error(h, msg, EINVAL, errmsg.c_str());
      return;
    }

    int user_id = json_integer_value(uid_j);
    int score = json_integer_value(score_j);

    scores[user_id] = score;
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
