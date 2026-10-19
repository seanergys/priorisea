// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/lib/job.hpp"
#include "src/lib/error.hpp"
#include <sys/types.h>

json_t *ps_job_to_json(const ps_job_t &job) {
  auto *obj = json_object();
  json_object_set_new(obj, "jobid", json_string(job.jobid.c_str()));
  json_object_set_new(obj, "userid", json_integer((json_int_t)job.userid));
  json_object_set_new(obj, "t_inactive", json_real(job.t_inactive));
  json_object_set_new(obj, "score", json_real(job.score));
  return obj;
}

ps_result_t<ps_job_t> ps_job_from_json(json_t *obj) {
  ps_job_t res;
  json_t *jobid_j = json_object_get(obj, "jobid");
  if (!jobid_j) {
    return ps_error(ps_field_missing_t{.field = "jobid"});
  }
  if (!json_is_string(jobid_j)) {
    return ps_error(
        ps_field_type_mismatch_t{.field = "jobid", .expected = "string"});
  }
  res.jobid = json_string_value(jobid_j);

  json_t *userid_j = json_object_get(obj, "userid");
  if (!userid_j) {
    return ps_error(ps_field_missing_t{.field = "userid"});
  }
  if (!json_is_integer(userid_j)) {
    return ps_error(
        ps_field_type_mismatch_t{.field = "userid", .expected = "integer"});
  }
  res.userid = static_cast<uid_t>(json_integer_value(userid_j));

  json_t *t_inactive_j = json_object_get(obj, "t_inactive");
  if (!t_inactive_j) {
    return ps_error(ps_field_missing_t{.field = "t_inactive"});
  }
  if (!json_is_real(t_inactive_j)) {
    return ps_error(
        ps_field_type_mismatch_t{.field = "t_inactive", .expected = "double"});
  }
  res.t_inactive = json_number_value(t_inactive_j);

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
