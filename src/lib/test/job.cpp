// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/lib/job.hpp"

#include <catch2/catch.hpp>

TEST_CASE("ps_job_from_json accepts well-formed input",
          "[job][ps_job_from_json]") {
  json_t *obj =
      json_pack("{s:s,s:i,s:f,s:f,s:f}", "jobid", "0x1234", "userid", 1000,
                "t_inactive", 42.5, "score", 7.25, "duration", 3.5);
  auto res = ps_job_from_json(obj);
  REQUIRE(ps_is_ok(res));
  REQUIRE(ps_unwrap(res).jobid == "0x1234");
  REQUIRE(ps_unwrap(res).userid == 1000);
  REQUIRE(ps_unwrap(res).t_inactive == Approx(42.5));
  REQUIRE(ps_unwrap(res).score == Approx(7.25));
  REQUIRE(ps_unwrap(res).duration == Approx(3.5));
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
}

TEST_CASE("ps_job_to_json and ps_job_from_json are inverse", "[job]") {
  ps_job_t job = {.jobid = "0x1234",
                  .userid = 1000,
                  .t_inactive = 42.5,
                  .score = 7.25,
                  .duration = 3.5};
  json_t *j = ps_job_to_json(job);
  auto res = ps_job_from_json(j);
  REQUIRE(ps_is_ok(res));
  auto back = ps_unwrap(res);
  REQUIRE(back.jobid == job.jobid);
  REQUIRE(back.userid == job.userid);
  REQUIRE(back.t_inactive == Approx(job.t_inactive));
  REQUIRE(back.score == Approx(job.score));
  REQUIRE(back.duration == Approx(job.duration));
  json_decref(j);
}
