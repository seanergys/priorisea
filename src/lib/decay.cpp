// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/lib/decay.hpp"
#include "src/lib/error.hpp"
#include "src/lib/util.hpp"
#include <flux/core.h>
#include <jansson.h>
#include <sstream>

json_t *ps_decay_frame_to_json(const ps_decay_frame_t &frame) {
  auto *obj = json_object();
  json_object_set_new(obj, "duration", json_integer(frame.duration));
  json_object_set_new(obj, "weight", json_real(frame.weight));
  return obj;
}

ps_result_t<ps_decay_frame_t> ps_decay_frame_from_json(json_t *obj) {
  ps_decay_frame_t res;
  json_t *duration_j = json_object_get(obj, "duration");
  if (!duration_j) {
    return ps_error(ps_field_missing_t{.field = "duration"});
  }
  if (!json_is_integer(duration_j)) {
    return ps_error(
        ps_field_type_mismatch_t{.field = "duration", .expected = "integer"});
  }
  res.duration = (int)json_integer_value(duration_j);
  json_t *weight_j = json_object_get(obj, "weight");
  if (!weight_j) {
    return ps_error(ps_field_missing_t{.field = "weight"});
  }
  if (!json_is_real(weight_j)) {
    return ps_error(
        ps_field_type_mismatch_t{.field = "weight", .expected = "double"});
  }
  res.weight = json_real_value(weight_j);
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
      return ps_error(
          ps_string_parse_err_t{.msg = "expected \":\" after \",\""});
    }
    auto duration_str = token.substr(0, colon_pos);
    auto weight_str = token.substr(colon_pos + 1);
    int duration;
    double weight;
    if (!parse_int(duration_str, duration)) {
      return ps_error(
          ps_field_type_mismatch_t{.field = "duration", .expected = "integer"});
    }
    if (!parse_double(weight_str, weight)) {
      return ps_error(
          ps_field_type_mismatch_t{.field = "weight", .expected = "double"});
    }
    res.push_back({.duration = duration, .weight = weight});
  }
  return res;
}
