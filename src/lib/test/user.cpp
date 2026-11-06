// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/lib/user.hpp"

#include <catch2/catch.hpp>

TEST_CASE("ps_user_from_json accepts well-formed input",
          "[user][ps_user_from_json]") {
  json_t *obj = json_pack("{s:i,s:f}", "userid", 1000, "score", 3.5);
  auto res = ps_user_from_json(obj);
  REQUIRE(ps_is_ok(res));
  REQUIRE(ps_unwrap(res).userid == 1000);
  REQUIRE(ps_unwrap(res).score.has_value());
  REQUIRE(ps_unwrap(res).score.value() == Approx(3.5));
  json_decref(obj);

  json_t *obj2 = json_pack("{s:i}", "userid", 1000);
  auto res2 = ps_user_from_json(obj2);
  REQUIRE(ps_is_ok(res2));
  REQUIRE(ps_unwrap(res2).userid == 1000);
  REQUIRE(!ps_unwrap(res2).score.has_value());
  json_decref(obj2);
}

TEST_CASE("ps_user_from_json rejects malformed input",
          "[user][ps_user_from_json]") {
  // Missing userid.
  json_t *o1 = json_pack("{}");
  auto r1 = ps_user_from_json(o1);
  REQUIRE_FALSE(ps_is_ok(r1));
  REQUIRE(ps_is_error_type<ps_field_missing_t>(r1));
  json_decref(o1);

  // userid present but not an integer.
  json_t *o2 = json_pack("{s:f}", "userid", 1.5);
  auto r2 = ps_user_from_json(o2);
  REQUIRE_FALSE(ps_is_ok(r2));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r2));
  json_decref(o2);

  // score present but not a real (integer is rejected).
  json_t *o3 = json_pack("{s:i,s:i}", "userid", 1000, "score", 3);
  auto r3 = ps_user_from_json(o3);
  REQUIRE_FALSE(ps_is_ok(r3));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r3));
  json_decref(o3);
}

TEST_CASE("ps_user_to_json and ps_user_from_json are inverse", "[user]") {
  ps_user_t user = {.userid = 1000, .score = 3.5};
  json_t *j = ps_user_to_json(user);
  auto res = ps_user_from_json(j);
  REQUIRE(ps_is_ok(res));
  REQUIRE(ps_unwrap(res).userid == 1000);
  REQUIRE(ps_unwrap(res).score.has_value());
  REQUIRE(ps_unwrap(res).score.value() == Approx(3.5));
  json_decref(j);

  ps_user_t user2 = {.userid = 1000};
  json_t *j2 = ps_user_to_json(user2);
  auto res2 = ps_user_from_json(j2);
  REQUIRE(ps_is_ok(res2));
  REQUIRE(ps_unwrap(res2).userid == 1000);
  REQUIRE(!ps_unwrap(res2).score.has_value());
  json_decref(j2);
}
