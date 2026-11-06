// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/lib/job.hpp"

#include <catch2/catch.hpp>

TEST_CASE("ps_job_from_json accepts well-formed input",
          "[job][ps_job_from_json]") {
  json_t *obj = json_pack(
      "{s:s,s:i,s:f,s:f,s:f,s:s,s:{s:f,s:f},s:{s:f,s:f},s:f,s:{s:f,s:f}}",
      "jobid", "0x1234", "userid", 1000, "t_inactive", 42.5, "score", 7.25,
      "duration", 3.5, "s_expr", "42", "s_defaults", "x", 1.0, "y", 2.0,
      "s_values", "x", 1.0, "y", 2.0, "weight", 3.0, "w_values", "x", 1.0, "y",
      2.0);
  auto res = ps_job_from_json(obj);
  REQUIRE(ps_is_ok(res));
  auto job = ps_unwrap(res);
  REQUIRE(job.jobid == "0x1234");
  REQUIRE(job.userid == 1000);
  REQUIRE(job.t_inactive == Approx(42.5));
  REQUIRE(job.score == Approx(7.25));
  REQUIRE(job.duration == Approx(3.5));
  REQUIRE(job.s_expr == "42");
  // Formula variable maps (required objects of name -> real).
  REQUIRE(job.s_defaults.size() == 2);
  REQUIRE(job.s_defaults.at("x") == Approx(1.0));
  REQUIRE(job.s_defaults.at("y") == Approx(2.0));
  REQUIRE(job.s_values.size() == 2);
  REQUIRE(job.s_values.at("x") == Approx(1.0));
  REQUIRE(job.s_values.at("y") == Approx(2.0));
  // Optional fields present.
  REQUIRE(job.weight.has_value());
  REQUIRE(*job.weight == Approx(3.0));
  REQUIRE(job.w_values.has_value());
  REQUIRE(job.w_values->at("x") == Approx(1.0));
  REQUIRE(job.w_values->at("y") == Approx(2.0));
  json_decref(obj);
}

TEST_CASE("ps_job_from_json omits optional fields", "[job][ps_job_from_json]") {
  json_t *obj = json_pack("{s:s,s:i,s:f,s:f,s:f,s:s,s:{s:f},s:{s:f}}", "jobid",
                          "0x1234", "userid", 1000, "t_inactive", 42.5, "score",
                          7.25, "duration", 3.5, "s_expr", "42", "s_defaults",
                          "x", 1.0, "s_values", "x", 1.0);
  auto res = ps_job_from_json(obj);
  REQUIRE(ps_is_ok(res));
  auto job = ps_unwrap(res);
  // Required formula maps still parsed.
  REQUIRE(job.s_defaults.size() == 1);
  REQUIRE(job.s_defaults.at("x") == Approx(1.0));
  REQUIRE(job.s_values.size() == 1);
  // Optional fields absent.
  REQUIRE_FALSE(job.weight.has_value());
  REQUIRE_FALSE(job.w_values.has_value());
  json_decref(obj);
}

TEST_CASE("ps_job_from_json rejects malformed input",
          "[job][ps_job_from_json]") {
  // Missing jobid.
  json_t *o1 =
      json_pack("{s:i,s:f,s:f}", "userid", 1, "t_inactive", 1.0, "score", 2.0);
  auto r1 = ps_job_from_json(o1);
  REQUIRE_FALSE(ps_is_ok(r1));
  REQUIRE(ps_is_error_type<ps_field_missing_t>(r1));
  json_decref(o1);

  // jobid present but not a string.
  json_t *o2 = json_pack("{s:i,s:i,s:f,s:f}", "jobid", 1, "userid", 1,
                         "t_inactive", 1.0, "score", 2.0);
  auto r2 = ps_job_from_json(o2);
  REQUIRE_FALSE(ps_is_ok(r2));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r2));
  json_decref(o2);

  // Missing userid.
  json_t *o3 = json_pack("{s:s,s:f,s:f}", "jobid", "0x1", "t_inactive", 1.0,
                         "score", 2.0);
  auto r3 = ps_job_from_json(o3);
  REQUIRE_FALSE(ps_is_ok(r3));
  REQUIRE(ps_is_error_type<ps_field_missing_t>(r3));
  json_decref(o3);

  // userid present but not an integer.
  json_t *o4 = json_pack("{s:s,s:f,s:f,s:f}", "jobid", "0x1", "userid", 1.5,
                         "t_inactive", 1.0, "score", 2.0);
  auto r4 = ps_job_from_json(o4);
  REQUIRE_FALSE(ps_is_ok(r4));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r4));
  json_decref(o4);

  // Missing t_inactive.
  json_t *o5 =
      json_pack("{s:s,s:i,s:f}", "jobid", "0x1", "userid", 1, "score", 2.0);
  auto r5 = ps_job_from_json(o5);
  REQUIRE_FALSE(ps_is_ok(r5));
  REQUIRE(ps_is_error_type<ps_field_missing_t>(r5));
  json_decref(o5);

  // t_inactive present but not a real (integer is rejected).
  json_t *o6 = json_pack("{s:s,s:i,s:i,s:f}", "jobid", "0x1", "userid", 1,
                         "t_inactive", 1, "score", 2.0);
  auto r6 = ps_job_from_json(o6);
  REQUIRE_FALSE(ps_is_ok(r6));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r6));
  json_decref(o6);

  // Missing score.
  json_t *o7 = json_pack("{s:s,s:i,s:f}", "jobid", "0x1", "userid", 1,
                         "t_inactive", 1.0);
  auto r7 = ps_job_from_json(o7);
  REQUIRE_FALSE(ps_is_ok(r7));
  REQUIRE(ps_is_error_type<ps_field_missing_t>(r7));
  json_decref(o7);

  // score present but not a real (integer is rejected).
  json_t *o8 = json_pack("{s:s,s:i,s:f,s:i}", "jobid", "0x1", "userid", 1,
                         "t_inactive", 1.0, "score", 2);
  auto r8 = ps_job_from_json(o8);
  REQUIRE_FALSE(ps_is_ok(r8));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r8));
  json_decref(o8);

  // Missing duration.
  json_t *o9 = json_pack("{s:s,s:i,s:f,s:f}", "jobid", "0x1", "userid", 1,
                         "t_inactive", 1.0, "score", 2.0);
  auto r9 = ps_job_from_json(o9);
  REQUIRE_FALSE(ps_is_ok(r9));
  REQUIRE(ps_is_error_type<ps_field_missing_t>(r9));
  json_decref(o9);

  // duration present but not a real (integer is rejected).
  json_t *o10 = json_pack("{s:s,s:i,s:f,s:f,s:i}", "jobid", "0x1", "userid", 1,
                          "t_inactive", 1.0, "score", 2.0, "duration", 3);
  auto r10 = ps_job_from_json(o10);
  REQUIRE_FALSE(ps_is_ok(r10));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r10));
  json_decref(o10);

  // Missing s_expr.
  json_t *o11 = json_pack("{s:s,s:i,s:f,s:f,s:f}", "jobid", "0x1", "userid", 1,
                          "t_inactive", 1.0, "score", 2.0, "duration", 3.0);
  auto r11 = ps_job_from_json(o11);
  REQUIRE_FALSE(ps_is_ok(r11));
  REQUIRE(ps_is_error_type<ps_field_missing_t>(r11));
  json_decref(o11);

  // s_expr present but not a string.
  json_t *o12 =
      json_pack("{s:s,s:i,s:f,s:f,s:f,s:i}", "jobid", "0x1", "userid", 1,
                "t_inactive", 1.0, "score", 2.0, "duration", 3.0, "s_expr", 42);
  auto r12 = ps_job_from_json(o12);
  REQUIRE_FALSE(ps_is_ok(r12));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r12));
  json_decref(o12);

  // Missing s_defaults.
  json_t *o13 = json_pack("{s:s,s:i,s:f,s:f,s:f,s:s}", "jobid", "0x1", "userid",
                          1, "t_inactive", 1.0, "score", 2.0, "duration", 3.0,
                          "s_expr", "42");
  auto r13 = ps_job_from_json(o13);
  REQUIRE_FALSE(ps_is_ok(r13));
  REQUIRE(ps_is_error_type<ps_field_missing_t>(r13));
  json_decref(o13);

  // s_defaults present but not an object.
  json_t *o14 = json_pack("{s:s,s:i,s:f,s:f,s:f,s:s,s:i}", "jobid", "0x1",
                          "userid", 1, "t_inactive", 1.0, "score", 2.0,
                          "duration", 3.0, "s_expr", "42", "s_defaults", 1);
  auto r14 = ps_job_from_json(o14);
  REQUIRE_FALSE(ps_is_ok(r14));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r14));
  json_decref(o14);

  // s_defaults an object but a value is not a real (integer rejected).
  json_t *o15 =
      json_pack("{s:s,s:i,s:f,s:f,s:f,s:s,s:{s:i}}", "jobid", "0x1", "userid",
                1, "t_inactive", 1.0, "score", 2.0, "duration", 3.0, "s_expr",
                "42", "s_defaults", "x", 1);
  auto r15 = ps_job_from_json(o15);
  REQUIRE_FALSE(ps_is_ok(r15));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r15));
  json_decref(o15);

  // Missing s_values.
  json_t *o16 =
      json_pack("{s:s,s:i,s:f,s:f,s:f,s:s,s:{s:f}}", "jobid", "0x1", "userid",
                1, "t_inactive", 1.0, "score", 2.0, "duration", 3.0, "s_expr",
                "42", "s_defaults", "x", 1.0);
  auto r16 = ps_job_from_json(o16);
  REQUIRE_FALSE(ps_is_ok(r16));
  REQUIRE(ps_is_error_type<ps_field_missing_t>(r16));
  json_decref(o16);

  // s_values present but not an object.
  json_t *o17 =
      json_pack("{s:s,s:i,s:f,s:f,s:f,s:s,s:{s:f},s:i}", "jobid", "0x1",
                "userid", 1, "t_inactive", 1.0, "score", 2.0, "duration", 3.0,
                "s_expr", "42", "s_defaults", "x", 1.0, "s_values", 1);
  auto r17 = ps_job_from_json(o17);
  REQUIRE_FALSE(ps_is_ok(r17));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r17));
  json_decref(o17);

  // Optional weight present but not a real (integer rejected).
  json_t *o18 = json_pack(
      "{s:s,s:i,s:f,s:f,s:f,s:s,s:{s:f},s:{s:f},s:i}", "jobid", "0x1", "userid",
      1, "t_inactive", 1.0, "score", 2.0, "duration", 3.0, "s_expr", "42",
      "s_defaults", "x", 1.0, "s_values", "x", 1.0, "weight", 2);
  auto r18 = ps_job_from_json(o18);
  REQUIRE_FALSE(ps_is_ok(r18));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r18));
  json_decref(o18);

  // Optional w_values present but not an object.
  json_t *o19 = json_pack(
      "{s:s,s:i,s:f,s:f,s:f,s:s,s:{s:f},s:{s:f},s:i}", "jobid", "0x1", "userid",
      1, "t_inactive", 1.0, "score", 2.0, "duration", 3.0, "s_expr", "42",
      "s_defaults", "x", 1.0, "s_values", "x", 1.0, "w_values", 1);
  auto r19 = ps_job_from_json(o19);
  REQUIRE_FALSE(ps_is_ok(r19));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r19));
  json_decref(o19);
}

TEST_CASE("ps_job_to_json and ps_job_from_json are inverse", "[job]") {
  // Without optional fields.
  ps_job_t job = {.jobid = "0x1234",
                  .userid = 1000,
                  .t_inactive = 42.5,
                  .score = 7.25,
                  .duration = 3.5,
                  .s_expr = "42"};
  json_t *j = ps_job_to_json(job);
  auto res = ps_job_from_json(j);
  REQUIRE(ps_is_ok(res));
  auto back = ps_unwrap(res);
  REQUIRE(back.jobid == job.jobid);
  REQUIRE(back.userid == job.userid);
  REQUIRE(back.t_inactive == Approx(job.t_inactive));
  REQUIRE(back.score == Approx(job.score));
  REQUIRE(back.duration == Approx(job.duration));
  REQUIRE(back.s_expr == job.s_expr);
  REQUIRE(back.s_defaults.size() == job.s_defaults.size());
  REQUIRE(back.s_values.size() == job.s_values.size());
  REQUIRE_FALSE(back.weight.has_value());
  REQUIRE_FALSE(back.w_values.has_value());
  json_decref(j);

  // With optional fields set.
  ps_job_t job2;
  job2.jobid = "0x5678";
  job2.userid = 2000;
  job2.t_inactive = 10.0;
  job2.score = 1.25;
  job2.duration = 2.0;
  job2.s_expr = "7";
  job2.s_defaults = {{"a", 1.0}, {"b", 2.0}};
  job2.s_values = {{"a", 3.0}};
  job2.weight = 4.5;
  job2.w_values = {{"a", 9.0}};
  json_t *j2 = ps_job_to_json(job2);
  auto res2 = ps_job_from_json(j2);
  REQUIRE(ps_is_ok(res2));
  auto back2 = ps_unwrap(res2);
  REQUIRE(back2.jobid == job2.jobid);
  REQUIRE(back2.userid == job2.userid);
  REQUIRE(back2.t_inactive == Approx(job2.t_inactive));
  REQUIRE(back2.score == Approx(job2.score));
  REQUIRE(back2.duration == Approx(job2.duration));
  REQUIRE(back2.s_expr == job2.s_expr);
  REQUIRE(back2.s_defaults.size() == 2);
  REQUIRE(back2.s_defaults.at("a") == Approx(1.0));
  REQUIRE(back2.s_defaults.at("b") == Approx(2.0));
  REQUIRE(back2.s_values.size() == 1);
  REQUIRE(back2.s_values.at("a") == Approx(3.0));
  REQUIRE(back2.weight.has_value());
  REQUIRE(*back2.weight == Approx(4.5));
  REQUIRE(back2.w_values.has_value());
  REQUIRE(back2.w_values->size() == 1);
  REQUIRE(back2.w_values->at("a") == Approx(9.0));
  json_decref(j2);
}
