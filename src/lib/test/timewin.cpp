// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/lib/timewin.hpp"

#include <catch2/catch.hpp>

TEST_CASE("ps_validate_cron accepts well-formed expressions",
          "[timewin][ps_validate_cron]") {
  // Six field expressions
  REQUIRE(ps_is_ok(ps_validate_cron("*/5 * * * * *")));
  REQUIRE(ps_is_ok(ps_validate_cron("0 0 * * * *")));
  REQUIRE(ps_is_ok(ps_validate_cron("* * * * * *")));
  REQUIRE(ps_is_ok(ps_validate_cron("0 0 * * 1-5 *")));

  // Seven field expressions
  REQUIRE(ps_is_ok(ps_validate_cron("*/5 * * * * * *")));
  REQUIRE(ps_is_ok(ps_validate_cron("0 0 * * * * *")));
  REQUIRE(ps_is_ok(ps_validate_cron("* * * * * * *")));
  REQUIRE(ps_is_ok(ps_validate_cron("0 0 * * 1-5 * *")));
}

TEST_CASE("ps_validate_cron rejects malformed input",
          "[timewin][ps_validate_cron]") {
  REQUIRE_FALSE(ps_is_ok(ps_validate_cron("notacron")));
  REQUIRE_FALSE(ps_is_ok(ps_validate_cron("*")));
  REQUIRE_FALSE(ps_is_ok(ps_validate_cron("")));
  REQUIRE_FALSE(ps_is_ok(ps_validate_cron("* * * * *")));
  REQUIRE_FALSE(ps_is_ok(ps_validate_cron("* * * * * * * *")));

  auto r = ps_validate_cron("notacron");
  REQUIRE(ps_is_error_type<ps_string_parse_err_t>(r));
}

TEST_CASE("ps_validate_timewin accepts well-formed input",
          "[timewin][ps_validate_timewin]") {
  auto res = ps_validate_timewin(
      {.name = "night", .cron = "0 0 * * * *", .duration = 3600, .score = 1.0});
  REQUIRE(ps_is_ok(res));
}

TEST_CASE("ps_validate_timewin rejects malformed input",
          "[timewin][ps_validate_timewin]") {
  // Empty name.
  auto r1 = ps_validate_timewin(
      {.name = "", .cron = "0 0 * * * *", .duration = 3600, .score = 1.0});
  REQUIRE_FALSE(ps_is_ok(r1));
  REQUIRE(ps_is_error_type<ps_field_missing_t>(r1));

  // Invalid cron expression.
  auto r2 = ps_validate_timewin(
      {.name = "x", .cron = "notacron", .duration = 3600, .score = 1.0});
  REQUIRE_FALSE(ps_is_ok(r2));
  REQUIRE(ps_is_error_type<ps_string_parse_err_t>(r2));

  // Negative duration.
  auto r3 = ps_validate_timewin(
      {.name = "x", .cron = "0 0 * * * *", .duration = -1, .score = 1.0});
  REQUIRE_FALSE(ps_is_ok(r3));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r3));
}

TEST_CASE("ps_timewin_from_json accepts well-formed input",
          "[timewin][ps_timewin_from_json]") {
  json_t *obj = json_pack("{s:s,s:s,s:i,s:f}", "name", "night", "cron",
                          "0 0 * * * *", "duration", 3600, "score", 1.5);
  auto res = ps_timewin_from_json(obj);
  REQUIRE(ps_is_ok(res));
  REQUIRE(ps_unwrap(res).name == "night");
  REQUIRE(ps_unwrap(res).cron == "0 0 * * * *");
  REQUIRE(ps_unwrap(res).duration == 3600);
  REQUIRE(ps_unwrap(res).score == Approx(1.5));
  json_decref(obj);
}

TEST_CASE("ps_timewin_from_json rejects malformed input",
          "[timewin][ps_timewin_from_json]") {
  // Missing name.
  json_t *o1 = json_pack("{s:s,s:i,s:f}", "cron", "0 0 * * * *", "duration",
                         3600, "score", 1.5);
  auto r1 = ps_timewin_from_json(o1);
  REQUIRE_FALSE(ps_is_ok(r1));
  REQUIRE(ps_is_error_type<ps_field_missing_t>(r1));
  json_decref(o1);

  // Name present but not a string.
  json_t *o2 = json_pack("{s:i,s:s,s:i,s:f}", "name", 42, "cron", "0 0 * * * *",
                         "duration", 3600, "score", 1.5);
  auto r2 = ps_timewin_from_json(o2);
  REQUIRE_FALSE(ps_is_ok(r2));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r2));
  json_decref(o2);

  // Missing cron.
  json_t *o3 =
      json_pack("{s:s,s:i,s:f}", "name", "n", "duration", 3600, "score", 1.5);
  auto r3 = ps_timewin_from_json(o3);
  REQUIRE_FALSE(ps_is_ok(r3));
  REQUIRE(ps_is_error_type<ps_field_missing_t>(r3));
  json_decref(o3);

  // Cron present but not a string.
  json_t *o4 = json_pack("{s:s,s:i,s:i,s:f}", "name", "n", "cron", 42,
                         "duration", 3600, "score", 1.5);
  auto r4 = ps_timewin_from_json(o4);
  REQUIRE_FALSE(ps_is_ok(r4));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r4));
  json_decref(o4);

  // Missing duration.
  json_t *o5 = json_pack("{s:s,s:s,s:f}", "name", "n", "cron", "0 0 * * * *",
                         "score", 1.5);
  auto r5 = ps_timewin_from_json(o5);
  REQUIRE_FALSE(ps_is_ok(r5));
  REQUIRE(ps_is_error_type<ps_field_missing_t>(r5));
  json_decref(o5);

  // Duration present but not an integer.
  json_t *o6 = json_pack("{s:s,s:s,s:f,s:f}", "name", "n", "cron",
                         "0 0 * * * *", "duration", 1.5, "score", 1.5);
  auto r6 = ps_timewin_from_json(o6);
  REQUIRE_FALSE(ps_is_ok(r6));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r6));
  json_decref(o6);

  // Missing score.
  json_t *o7 = json_pack("{s:s,s:s,s:i}", "name", "n", "cron", "0 0 * * * *",
                         "duration", 3600);
  auto r7 = ps_timewin_from_json(o7);
  REQUIRE_FALSE(ps_is_ok(r7));
  REQUIRE(ps_is_error_type<ps_field_missing_t>(r7));
  json_decref(o7);

  // Score present but not a real (integer is rejected).
  json_t *o8 = json_pack("{s:s,s:s,s:i,s:i}", "name", "n", "cron",
                         "0 0 * * * *", "duration", 3600, "score", 2);
  auto r8 = ps_timewin_from_json(o8);
  REQUIRE_FALSE(ps_is_ok(r8));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r8));
  json_decref(o8);
}

TEST_CASE("ps_timewin_to_json and ps_timewin_from_json are inverse",
          "[timewin]") {
  ps_timewin_t timewin = {
      .name = "night", .cron = "0 0 * * * *", .duration = 3600, .score = 1.5};
  json_t *j = ps_timewin_to_json(timewin);
  auto res = ps_timewin_from_json(j);
  REQUIRE(ps_is_ok(res));
  auto back = ps_unwrap(res);
  REQUIRE(back.name == timewin.name);
  REQUIRE(back.cron == timewin.cron);
  REQUIRE(back.duration == timewin.duration);
  REQUIRE(back.score == Approx(timewin.score));
  json_decref(j);
}
