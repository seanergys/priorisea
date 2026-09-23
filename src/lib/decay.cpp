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

ps_decay_frame_t ps_decay_frame_from_json(json_t *obj) {
  ps_decay_frame_t res;
  int duration = 0;
  double priority = 0;
  json_unpack(obj, "{s:i, s:f}", "duration", &duration, "priority", &priority);
  res.duration = duration;
  res.priority = priority;
  return res;
}

json_t *ps_decay_to_json(const ps_decay_t &decay) {
  auto *arr = json_array();
  for (const auto &d : decay) {
    json_array_append_new(arr, ps_decay_frame_to_json(d));
  }
  return arr;
}

ps_decay_t ps_decay_from_json(json_t *obj) {
  ps_decay_t res;
  size_t index;
  json_t *value;
  json_array_foreach(obj, index, value) {
    res.push_back(ps_decay_frame_from_json(value));
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
    return ps_forward_error<ps_decay_t>(lookup_res,
                                        "getting stored decay value");
  }
  auto decay_str = ps_unwrap(lookup_res);

  json_error_t json_err;
  auto *json_decay = json_loads(decay_str.c_str(), 0, &json_err);
  if (!json_decay) {
    return ps_error(ps_json_unpack_err_t{.dumps = decay_str},
                    "parsing stored decay value");
  }

  auto decay = ps_decay_from_json(json_decay);
  json_decref(json_decay);
  return decay;
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
