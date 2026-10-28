// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/lib/info.hpp"

#include <catch2/catch.hpp>

TEST_CASE("ps_db_info_from_json accepts well-formed input",
          "[info][ps_db_info_from_json]") {
  json_t *obj =
      json_pack("{s:f,s:f}", "t_inactive", 100.0, "user_period", 86400.0);
  auto res = ps_db_info_from_json(obj);
  REQUIRE(ps_is_ok(res));
  REQUIRE(ps_unwrap(res).t_inactive == Approx(100.0));
  REQUIRE(ps_unwrap(res).user_period == Approx(86400.0));
  json_decref(obj);
}

TEST_CASE("ps_db_info_from_json rejects malformed input",
          "[info][ps_db_info_from_json]") {
  // Missing t_inactive.
  json_t *o1 = json_pack("{s:f}", "user_period", 86400.0);
  auto r1 = ps_db_info_from_json(o1);
  REQUIRE_FALSE(ps_is_ok(r1));
  REQUIRE(ps_is_error_type<ps_field_missing_t>(r1));
  json_decref(o1);

  // t_inactive present but not a real (integer is rejected).
  json_t *o2 =
      json_pack("{s:i,s:f}", "t_inactive", 100, "user_period", 86400.0);
  auto r2 = ps_db_info_from_json(o2);
  REQUIRE_FALSE(ps_is_ok(r2));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r2));
  json_decref(o2);

  // Missing user_period.
  json_t *o3 = json_pack("{s:f}", "t_inactive", 100.0);
  auto r3 = ps_db_info_from_json(o3);
  REQUIRE_FALSE(ps_is_ok(r3));
  REQUIRE(ps_is_error_type<ps_field_missing_t>(r3));
  json_decref(o3);

  // user_period present but not a real (integer is rejected).
  json_t *o4 =
      json_pack("{s:f,s:i}", "t_inactive", 100.0, "user_period", 86400);
  auto r4 = ps_db_info_from_json(o4);
  REQUIRE_FALSE(ps_is_ok(r4));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r4));
  json_decref(o4);
}

TEST_CASE("ps_db_info_to_json and ps_db_info_from_json are inverse", "[info]") {
  ps_db_info_t info = {.t_inactive = 100.0, .user_period = 86400.0};
  json_t *j = ps_db_info_to_json(info);
  REQUIRE(json_is_object(j));
  auto res = ps_db_info_from_json(j);
  REQUIRE(ps_is_ok(res));
  auto back = ps_unwrap(res);
  REQUIRE(back.t_inactive == Approx(info.t_inactive));
  REQUIRE(back.user_period == Approx(info.user_period));
  json_decref(j);
}
