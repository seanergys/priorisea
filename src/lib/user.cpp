// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/lib/user.hpp"
#include "src/lib/error.hpp"
#include <sys/types.h>

json_t *ps_user_to_json(const ps_user_t &user) {
  auto *obj = json_object();
  json_object_set_new(obj, "userid", json_integer((json_int_t)user.userid));
  json_object_set_new(obj, "score", json_real(user.score));
  return obj;
}

ps_result_t<ps_user_t> ps_user_from_json(json_t *obj) {
  ps_user_t res;
  json_t *userid_j = json_object_get(obj, "userid");
  if (!userid_j) {
    return ps_error(ps_field_missing_t{.field = "userid"});
  }
  if (!json_is_integer(userid_j)) {
    return ps_error(
        ps_field_type_mismatch_t{.field = "userid", .expected = "integer"});
  }
  res.userid = static_cast<uid_t>(json_integer_value(userid_j));
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
