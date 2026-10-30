#include "src/lib/job.hpp"
#include "src/lib/error.hpp"
#include "src/lib/formula.hpp"
#include <sys/types.h>

json_t *ps_job_to_json(const ps_job_t &job) {
  auto *obj = json_object();
  json_object_set_new(obj, "jobid", json_string(job.jobid.c_str()));
  json_object_set_new(obj, "userid", json_integer((json_int_t)job.userid));
  json_object_set_new(obj, "t_inactive", json_real(job.t_inactive));
  json_object_set_new(obj, "score", json_real(job.score));
  json_object_set_new(obj, "duration", json_real(job.duration));
  json_object_set_new(obj, "s_expr", json_string(job.s_expr.c_str()));
  json_object_set_new(obj, "s_values", ps_formula_vars_to_json(job.s_values));
  json_object_set_new(obj, "s_defaults",
                      ps_formula_vars_to_json(job.s_defaults));
  return obj;
}

json_t *ps_job_to_json_plain(const ps_job_t &job) {
  auto *obj = json_object();
  json_object_set_new(obj, "jobid", json_string(job.jobid.c_str()));
  json_object_set_new(obj, "userid", json_integer((json_int_t)job.userid));
  json_object_set_new(obj, "t_inactive", json_real(job.t_inactive));
  json_object_set_new(obj, "score", json_real(job.score));
  json_object_set_new(obj, "duration", json_real(job.duration));
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

  json_t *duration_j = json_object_get(obj, "duration");
  if (!duration_j) {
    return ps_error(ps_field_missing_t{.field = "duration"});
  }
  if (!json_is_real(duration_j)) {
    return ps_error(
        ps_field_type_mismatch_t{.field = "duration", .expected = "double"});
  }
  res.duration = json_number_value(duration_j);

  json_t *s_expr_j = json_object_get(obj, "s_expr");
  if (s_expr_j) {
    if (!json_is_string(s_expr_j)) {
      return ps_error(
          ps_field_type_mismatch_t{.field = "s_expr", .expected = "string"});
    }
    res.s_expr = json_string_value(s_expr_j);
  }

  json_t *defaults_obj = json_object_get(obj, "s_defaults");
  if (defaults_obj) {
    if (!json_is_object(defaults_obj)) {
      return ps_error(ps_field_type_mismatch_t{.field = "s_defaults",
                                               .expected = "object"});
    }
    auto defaults_res = ps_formula_vars_from_json(defaults_obj);
    if (!ps_is_ok(defaults_res)) {
      return ps_forward_error<ps_job_t>(defaults_res);
    }
    res.s_defaults = ps_unwrap(defaults_res);
  }

  json_t *values_obj = json_object_get(obj, "s_values");
  if (values_obj) {
    if (!json_is_object(values_obj)) {
      return ps_error(
          ps_field_type_mismatch_t{.field = "s_values", .expected = "object"});
    }
    auto values_res = ps_formula_vars_from_json(values_obj);
    if (!ps_is_ok(values_res)) {
      return ps_forward_error<ps_job_t>(values_res);
    }
    res.s_values = ps_unwrap(values_res);
  }
  return res;
}
