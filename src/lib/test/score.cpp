// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "lib/score.hpp"

#include <catch2/catch.hpp>

TEST_CASE("ps_job_from_json accepts well-formed input",
          "[score][ps_job_from_json]") {
  json_t *obj = json_pack("{s:i,s:f,s:f}", "jobid", 12345, "t_inactive", 42.5,
                          "score", 7.25);
  auto res = ps_job_from_json(obj);
  REQUIRE(ps_is_ok(res));
  REQUIRE(ps_unwrap(res).jobid == 12345);
  REQUIRE(ps_unwrap(res).t_inactive == Approx(42.5));
  REQUIRE(ps_unwrap(res).score == Approx(7.25));
  json_decref(obj);
}

TEST_CASE("ps_job_from_json rejects malformed input",
          "[score][ps_job_from_json]") {
  // Missing jobid.
  json_t *o1 = json_pack("{s:f,s:f}", "t_inactive", 1.0, "score", 2.0);
  auto r1 = ps_job_from_json(o1);
  REQUIRE_FALSE(ps_is_ok(r1));
  REQUIRE(ps_is_error_type<ps_job_jobid_missing_t>(r1));
  json_decref(o1);

  // jobid present but not an integer.
  json_t *o2 =
      json_pack("{s:f,s:f,s:f}", "jobid", 1.5, "t_inactive", 1.0, "score", 2.0);
  auto r2 = ps_job_from_json(o2);
  REQUIRE_FALSE(ps_is_ok(r2));
  REQUIRE(ps_is_error_type<ps_job_jobid_invalid_t>(r2));
  json_decref(o2);

  // Missing t_inactive.
  json_t *o3 = json_pack("{s:i,s:f}", "jobid", 1, "score", 2.0);
  auto r3 = ps_job_from_json(o3);
  REQUIRE_FALSE(ps_is_ok(r3));
  REQUIRE(ps_is_error_type<ps_job_t_inactive_missing_t>(r3));
  json_decref(o3);

  // t_inactive present but not a real (integer is rejected).
  json_t *o4 =
      json_pack("{s:i,s:i,s:f}", "jobid", 1, "t_inactive", 1, "score", 2.0);
  auto r4 = ps_job_from_json(o4);
  REQUIRE_FALSE(ps_is_ok(r4));
  REQUIRE(ps_is_error_type<ps_job_t_inactive_invalid_t>(r4));
  json_decref(o4);

  // Missing score.
  json_t *o5 = json_pack("{s:i,s:f}", "jobid", 1, "t_inactive", 1.0);
  auto r5 = ps_job_from_json(o5);
  REQUIRE_FALSE(ps_is_ok(r5));
  REQUIRE(ps_is_error_type<ps_job_score_missing_t>(r5));
  json_decref(o5);

  // score present but not a real (integer is rejected).
  json_t *o6 =
      json_pack("{s:i,s:f,s:i}", "jobid", 1, "t_inactive", 1.0, "score", 2);
  auto r6 = ps_job_from_json(o6);
  REQUIRE_FALSE(ps_is_ok(r6));
  REQUIRE(ps_is_error_type<ps_job_score_invalid_t>(r6));
  json_decref(o6);
}

TEST_CASE("ps_job_to_json and ps_job_from_json are inverse", "[score]") {
  ps_job_t job = {.jobid = 12345, .t_inactive = 42.5, .score = 7.25};
  json_t *j = ps_job_to_json(job);
  auto res = ps_job_from_json(j);
  REQUIRE(ps_is_ok(res));
  auto back = ps_unwrap(res);
  REQUIRE(back.jobid == job.jobid);
  REQUIRE(back.t_inactive == Approx(job.t_inactive));
  REQUIRE(back.score == Approx(job.score));
  json_decref(j);
}

TEST_CASE("ps_user_from_json accepts well-formed input",
          "[score][ps_user_from_json]") {
  json_t *obj = json_pack("{s:i,s:f}", "userid", 1000, "score", 3.5);
  auto res = ps_user_from_json(obj);
  REQUIRE(ps_is_ok(res));
  REQUIRE(ps_unwrap(res).userid == 1000);
  REQUIRE(ps_unwrap(res).score == Approx(3.5));
  json_decref(obj);
}

TEST_CASE("ps_user_from_json rejects malformed input",
          "[score][ps_user_from_json]") {
  // Missing userid.
  json_t *o1 = json_pack("{s:f}", "score", 3.5);
  auto r1 = ps_user_from_json(o1);
  REQUIRE_FALSE(ps_is_ok(r1));
  REQUIRE(ps_is_error_type<ps_user_userid_missing_t>(r1));
  json_decref(o1);

  // userid present but not an integer.
  json_t *o2 = json_pack("{s:f,s:f}", "userid", 1.5, "score", 3.5);
  auto r2 = ps_user_from_json(o2);
  REQUIRE_FALSE(ps_is_ok(r2));
  REQUIRE(ps_is_error_type<ps_user_userid_invalid_t>(r2));
  json_decref(o2);

  // Missing score.
  json_t *o3 = json_pack("{s:i}", "userid", 1000);
  auto r3 = ps_user_from_json(o3);
  REQUIRE_FALSE(ps_is_ok(r3));
  REQUIRE(ps_is_error_type<ps_user_score_missing_t>(r3));
  json_decref(o3);

  // score present but not a real (integer is rejected).
  json_t *o4 = json_pack("{s:i,s:i}", "userid", 1000, "score", 3);
  auto r4 = ps_user_from_json(o4);
  REQUIRE_FALSE(ps_is_ok(r4));
  REQUIRE(ps_is_error_type<ps_user_score_invalid_t>(r4));
  json_decref(o4);
}

TEST_CASE("ps_user_to_json and ps_user_from_json are inverse", "[score]") {
  ps_user_t user = {.userid = 1000, .score = 3.5};
  json_t *j = ps_user_to_json(user);
  auto res = ps_user_from_json(j);
  REQUIRE(ps_is_ok(res));
  REQUIRE(ps_unwrap(res).userid == 1000);
  REQUIRE(ps_unwrap(res).score == Approx(3.5));
  json_decref(j);
}

TEST_CASE("score_map_from_json accepts well-formed input",
          "[score][score_map_from_json]") {
  json_t *obj = json_pack("{s:f,s:f}", "1000", 1.5, "1001", 2.5);
  auto res = score_map_from_json(obj);
  REQUIRE(ps_is_ok(res));
  auto m = ps_unwrap(res);
  REQUIRE(m.size() == 2);
  REQUIRE(m.at(1000) == Approx(1.5));
  REQUIRE(m.at(1001) == Approx(2.5));
  json_decref(obj);

  // Keys that are not valid uids are skipped rather than rejected.
  json_t *obj2 = json_pack("{s:f,s:f}", "1000", 3.0, "not_a_uid", 9.0);
  auto res2 = score_map_from_json(obj2);
  REQUIRE(ps_is_ok(res2));
  REQUIRE(ps_unwrap(res2).size() == 1);
  REQUIRE(ps_unwrap(res2).at(1000) == Approx(3.0));
  json_decref(obj2);
}

TEST_CASE("score_map_from_json rejects malformed input",
          "[score][score_map_from_json]") {
  // Not a JSON object.
  json_t *o1 = json_array();
  auto r1 = score_map_from_json(o1);
  REQUIRE_FALSE(ps_is_ok(r1));
  REQUIRE(ps_is_error_type<ps_score_map_invalid_t>(r1));
  json_decref(o1);

  // A value that is not a number.
  json_t *o2 = json_pack("{s:s}", "1000", "oops");
  auto r2 = score_map_from_json(o2);
  REQUIRE_FALSE(ps_is_ok(r2));
  REQUIRE(ps_is_error_type<ps_score_map_value_invalid_t>(r2));
  json_decref(o2);
}

TEST_CASE("score_map_to_json and score_map_from_json are inverse", "[score]") {
  ps_score_map_t scores = {{1000, 1.5}, {1001, 2.5}};
  json_t *j = score_map_to_json(scores);
  REQUIRE(json_is_object(j));
  auto res = score_map_from_json(j);
  REQUIRE(ps_is_ok(res));
  auto back = ps_unwrap(res);
  REQUIRE(back.size() == scores.size());
  REQUIRE(back.at(1000) == Approx(1.5));
  REQUIRE(back.at(1001) == Approx(2.5));
  json_decref(j);
}
