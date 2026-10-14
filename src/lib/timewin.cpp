// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/lib/timewin.hpp"
#include "src/lib/error.hpp"
#include <cerrno>
#include <croncpp.h>
#include <flux/core.h>
#include <jansson.h>

ps_result_t<ps_success_t> ps_validate_cron(const std::string &expr) {
  try {
    cron::make_cron(expr);
    return ps_success_t{};
  } catch (cron::bad_cronexpr const &ex) {
    return ps_error(ps_string_parse_err_t{.msg = ex.what()});
  }
}

ps_result_t<ps_success_t> ps_validate_timewin(const ps_timewin_t &timewin) {
  if (timewin.name.empty()) {
    return ps_error(ps_field_missing_t{.field = "name"});
  }
  auto cron_res = ps_validate_cron(timewin.cron);
  if (!ps_is_ok(cron_res)) {
    return cron_res;
  }
  if (timewin.duration < 0) {
    return ps_error(ps_field_type_mismatch_t{.field = "duration",
                                             .expected = "positive integer"});
  }
  return ps_success_t{};
}

json_t *ps_timewin_to_json(const ps_timewin_t &timewin) {
  auto *obj = json_object();
  json_object_set_new(obj, "name", json_string(timewin.name.c_str()));
  json_object_set_new(obj, "cron", json_string(timewin.cron.c_str()));
  json_object_set_new(obj, "duration", json_integer(timewin.duration));
  json_object_set_new(obj, "score", json_real(timewin.score));
  return obj;
}

ps_result_t<ps_timewin_t> ps_timewin_from_json(json_t *obj) {
  ps_timewin_t res;
  json_t *name_j = json_object_get(obj, "name");
  if (!name_j) {
    return ps_error(ps_field_missing_t{.field = "name"});
  }
  if (!json_is_string(name_j)) {
    return ps_error(
        ps_field_type_mismatch_t{.field = "name", .expected = "string"});
  }
  res.name = json_string_value(name_j);

  json_t *cron_j = json_object_get(obj, "cron");
  if (!cron_j) {
    return ps_error(ps_field_missing_t{.field = "cron"});
  }
  if (!json_is_string(cron_j)) {
    return ps_error(
        ps_field_type_mismatch_t{.field = "cron", .expected = "string"});
  }
  res.cron = json_string_value(cron_j);

  json_t *duration_j = json_object_get(obj, "duration");
  if (!duration_j) {
    return ps_error(ps_field_missing_t{.field = "duration"});
  }
  if (!json_is_integer(duration_j)) {
    return ps_error(
        ps_field_type_mismatch_t{.field = "duration", .expected = "integer"});
  }
  res.duration = static_cast<int>(json_integer_value(duration_j));

  json_t *score_j = json_object_get(obj, "score");
  if (!score_j) {
    return ps_error(ps_field_missing_t{.field = "score"});
  }
  if (!json_is_real(score_j)) {
    return ps_error(
        ps_field_type_mismatch_t{.field = "score", .expected = "double"});
  }
  res.score = json_number_value(score_j);
  return res;
}
