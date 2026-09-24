// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "decay.hpp"
#include "error.hpp"
#include "flux.hpp"
#include "util.hpp"
#include <flux/core.h>
#include <jansson.h>
#include <sstream>

static const ps_decay_t DEFAULT_DECAY = {
    {.duration = 86400, .priority = 1},
    {.duration = 604800, .priority = 0.5},
    {.duration = 1209600, .priority = 0.25},
};

////

json_t *ps_decay_frame_to_json(const ps_decay_frame_t &frame) {
  auto *obj = json_object();
  json_object_set_new(obj, "duration", json_integer(frame.duration));
  json_object_set_new(obj, "priority", json_real(frame.priority));
  return obj;
}

ps_result_t<ps_decay_frame_t> ps_decay_frame_from_json(json_t *obj) {
  ps_decay_frame_t res;
  json_t *duration_j = json_object_get(obj, "duration");
  if (!duration_j) {
    return ps_error(ps_decay_frame_duration_missing_t{});
  }
  if (!json_is_integer(duration_j)) {
    return ps_error(ps_decay_frame_duration_invalid_t{});
  }
  res.duration = (int)json_integer_value(duration_j);
  json_t *priority_j = json_object_get(obj, "priority");
  if (!priority_j) {
    return ps_error(ps_decay_priority_missing_t{});
  }
  if (!json_is_number(priority_j)) {
    return ps_error(ps_decay_priority_invalid_t{});
  }
  res.priority = json_number_value(priority_j);
  return res;
}

json_t *ps_decay_to_json(const ps_decay_t &decay) {
  auto *arr = json_array();
  for (const auto &d : decay) {
    json_array_append_new(arr, ps_decay_frame_to_json(d));
  }
  return arr;
}

ps_result_t<ps_decay_t> ps_decay_from_json(json_t *obj) {
  ps_decay_t res;
  size_t index;
  json_t *value;
  json_array_foreach(obj, index, value) {
    auto frame_res = ps_decay_frame_from_json(value);
    if (!ps_is_ok(frame_res)) {
      return ps_forward_error<ps_decay_t>(frame_res, "parsing decay frame");
    }
    res.push_back(ps_unwrap(frame_res));
  }
  return res;
}

ps_result_t<ps_decay_t> ps_decay_from_string(const std::string &expr) {
  ps_decay_t res;
  std::string token;
  std::istringstream stream(expr);
  while (std::getline(stream, token, ',')) {
    auto colon_pos = token.find(':');
    if (colon_pos == std::string::npos) {
      return ps_error(ps_decay_string_parse_err_t{.expr = expr});
    }
    auto duration_str = token.substr(0, colon_pos);
    auto weight_str = token.substr(colon_pos + 1);
    int duration;
    double priority;
    if (!parse_int(duration_str, duration) ||
        !parse_double(weight_str, priority)) {
      return ps_error(ps_decay_string_parse_err_t{.expr = expr});
    }
    res.push_back({.duration = duration, .priority = priority});
  }
  if (res.empty()) {
    return ps_error(ps_decay_string_parse_err_t{.expr = expr});
  }
  return res;
}

ps_result_t<ps_success_t> ps_db_decay_set(const ps_decay_t &decay) {
  auto *json_decay = ps_decay_to_json(decay);
  char *decay_str = json_dumps(json_decay, 0);
  json_decref(json_decay);
  if (!decay_str) {
    return ps_error(ps_json_pack_err_t{});
  }

  auto txn = flux_kvs_txn_create();
  if (!txn) {
    free(decay_str);
    return ps_error(ps_flux_kvs_txn_create_err_t{.msg = strerror(errno)});
  }
  auto key = "priorisea.decay";
  if (flux_kvs_txn_put(txn, 0, key, decay_str) < 0) {
    free(decay_str);
    return ps_error(ps_flux_kvs_txn_put_err_t{
        .key = key, .value = decay_str, .msg = strerror(errno)});
  }
  free(decay_str);
  return ps_flux_kvs_commit(txn);
}

ps_result_t<ps_decay_t> ps_db_decay_get() {
  auto lookup_res = ps_flux_kvs_lookup("priorisea.decay");
  if (!ps_is_ok(lookup_res)) {
    return ps_forward_error<ps_decay_t>(
        lookup_res, "No decay value stored. Did you run `priorisea init`?");
  }
  auto decay_str = ps_unwrap(lookup_res);

  json_error_t json_err;
  auto *json_decay = json_loads(decay_str.c_str(), 0, &json_err);
  if (!json_decay) {
    return ps_error(ps_json_unpack_err_t{.dumps = decay_str},
                    "parsing stored decay value");
  }

  auto decay_res = ps_decay_from_json(json_decay);
  json_decref(json_decay);
  if (!ps_is_ok(decay_res)) {
    return ps_add_context(decay_res, "parsing stored decay value");
  }
  return ps_unwrap(decay_res);
}

bool ps_db_is_decay() { return ps_is_ok(ps_db_decay_get()); }

ps_result_t<ps_success_t> ps_db_decay_reset() {
  return ps_db_decay_set(DEFAULT_DECAY);
}

ps_result_t<ps_success_t> ps_db_decay_init() {
  if (!ps_db_is_decay()) {
    return ps_db_decay_reset();
  }
  return ps_success_t{};
}
