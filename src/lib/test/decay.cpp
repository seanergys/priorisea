// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "lib/decay.hpp"

#include <catch2/catch.hpp>

TEST_CASE("ps_decay_frame_from_json accepts well-formed input",
          "[decay][ps_decay_frame_from_json]") {
  json_t *obj = json_pack("{s:i,s:f}", "duration", 86400, "weight", 0.5);
  auto res = ps_decay_frame_from_json(obj);
  REQUIRE(ps_is_ok(res));
  REQUIRE(ps_unwrap(res).duration == 86400);
  REQUIRE(ps_unwrap(res).weight == Approx(0.5));
  json_decref(obj);
}

TEST_CASE("ps_decay_frame_from_json rejects malformed input",
          "[decay][ps_decay_frame_from_json]") {
  // Missing duration.
  json_t *o1 = json_object();
  auto r1 = ps_decay_frame_from_json(o1);
  REQUIRE_FALSE(ps_is_ok(r1));
  REQUIRE(ps_is_error_type<ps_decay_frame_duration_missing_t>(r1));
  json_decref(o1);

  // Duration present but not an integer.
  json_t *o2 = json_pack("{s:f,s:f}", "duration", 1.5, "weight", 0.5);
  auto r2 = ps_decay_frame_from_json(o2);
  REQUIRE_FALSE(ps_is_ok(r2));
  REQUIRE(ps_is_error_type<ps_decay_frame_duration_invalid_t>(r2));
  json_decref(o2);

  // Missing weight.
  json_t *o3 = json_pack("{s:i}", "duration", 86400);
  auto r3 = ps_decay_frame_from_json(o3);
  REQUIRE_FALSE(ps_is_ok(r3));
  REQUIRE(ps_is_error_type<ps_decay_weight_missing_t>(r3));
  json_decref(o3);

  // Weight present but not a number.
  json_t *o4 = json_pack("{s:i,s:s}", "duration", 86400, "weight", "x");
  auto r4 = ps_decay_frame_from_json(o4);
  REQUIRE_FALSE(ps_is_ok(r4));
  REQUIRE(ps_is_error_type<ps_decay_weight_invalid_t>(r4));
  json_decref(o4);
}

TEST_CASE("ps_decay_frame_to_json and ps_decay_frame_from_json are inverse",
          "[decay]") {
  ps_decay_frame_t frame = {.duration = 86400, .weight = 0.5};
  json_t *j = ps_decay_frame_to_json(frame);
  auto res = ps_decay_frame_from_json(j);
  REQUIRE(ps_is_ok(res));
  REQUIRE(ps_unwrap(res).duration == 86400);
  REQUIRE(ps_unwrap(res).weight == Approx(0.5));
  json_decref(j);
}

TEST_CASE("ps_decay_from_json accepts well-formed input",
          "[decay][ps_decay_from_json]") {
  json_t *arr = json_pack("[{s:i,s:f},{s:i,s:f}]", "duration", 86400, "weight",
                          1.0, "duration", 604800, "weight", 0.5);
  auto res = ps_decay_from_json(arr);
  REQUIRE(ps_is_ok(res));
  auto d = ps_unwrap(res);
  REQUIRE(d.size() == 2);
  REQUIRE(d[0].duration == 86400);
  REQUIRE(d[0].weight == Approx(1.0));
  REQUIRE(d[1].duration == 604800);
  REQUIRE(d[1].weight == Approx(0.5));
  json_decref(arr);

  // An empty array yields an empty (but valid) decay.
  json_t *empty = json_array();
  auto res2 = ps_decay_from_json(empty);
  REQUIRE(ps_is_ok(res2));
  REQUIRE(ps_unwrap(res2).empty());
  json_decref(empty);
}

TEST_CASE("ps_decay_from_json rejects malformed input",
          "[decay][ps_decay_from_json]") {
  // A frame missing its duration.
  json_t *o1 = json_pack("[{s:f}]", "weight", 1.0);
  auto r1 = ps_decay_from_json(o1);
  REQUIRE_FALSE(ps_is_ok(r1));
  REQUIRE(ps_is_error_type<ps_decay_frame_duration_missing_t>(r1));
  json_decref(o1);

  // A frame whose duration is not an integer.
  json_t *o2 = json_pack("[{s:s,s:f}]", "duration", "x", "weight", 1.0);
  auto r2 = ps_decay_from_json(o2);
  REQUIRE_FALSE(ps_is_ok(r2));
  REQUIRE(ps_is_error_type<ps_decay_frame_duration_invalid_t>(r2));
  json_decref(o2);
}

TEST_CASE("ps_decay_to_json and ps_decay_from_json are inverse", "[decay]") {
  ps_decay_t decay = {
      {.duration = 86400, .weight = 1},
      {.duration = 604800, .weight = 0.5},
      {.duration = 1209600, .weight = 0.25},
  };
  json_t *j = ps_decay_to_json(decay);
  REQUIRE(json_is_array(j));
  auto res = ps_decay_from_json(j);
  REQUIRE(ps_is_ok(res));
  auto back = ps_unwrap(res);
  REQUIRE(back.size() == decay.size());
  for (size_t i = 0; i < decay.size(); ++i) {
    REQUIRE(back[i].duration == decay[i].duration);
    REQUIRE(back[i].weight == Approx(decay[i].weight));
  }
  json_decref(j);
}

TEST_CASE("ps_decay_from_string accepts well-formed input",
          "[decay][ps_decay_from_json]") {
  auto res = ps_decay_from_string("86400:1,604800:0.5");
  REQUIRE(ps_is_ok(res));
  auto d = ps_unwrap(res);
  REQUIRE(d.size() == 2);
  REQUIRE(d[0].duration == 86400);
  REQUIRE(d[0].weight == Approx(1.0));
  REQUIRE(d[1].duration == 604800);
  REQUIRE(d[1].weight == Approx(0.5));

  // A single frame is also valid.
  auto res2 = ps_decay_from_string("86400:1");
  REQUIRE(ps_is_ok(res2));
  REQUIRE(ps_unwrap(res2).size() == 1);
}

TEST_CASE("ps_decay_from_string rejects malformed input",
          "[decay][ps_decay_from_string]") {
  // Empty string.
  REQUIRE_FALSE(ps_is_ok(ps_decay_from_string("")));

  // Token without a colon separator.
  REQUIRE_FALSE(ps_is_ok(ps_decay_from_string("86400")));

  // Non-integer duration.
  REQUIRE_FALSE(ps_is_ok(ps_decay_from_string("x:1")));

  // Non-numeric weight.
  REQUIRE_FALSE(ps_is_ok(ps_decay_from_string("86400:x")));

  auto r = ps_decay_from_string("86400");
  REQUIRE(ps_is_error_type<ps_decay_string_parse_err_t>(r));
}
