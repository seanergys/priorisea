#include "src/lib/user.hpp"
#include "src/lib/error.hpp"
#include "src/lib/formula.hpp"
#include <jansson.h>
#include <sys/types.h>

json_t *ps_user_to_json(const ps_user_t &user) {
  auto *obj = json_object();
  json_object_set_new(obj, "userid", json_integer((json_int_t)user.userid));
  if (user.score) {
    json_object_set_new(obj, "score", json_real(*user.score));
  }
  json_object_set_new(obj, "w_expr", json_string(user.w_expr.c_str()));
  json_object_set_new(obj, "w_defaults",
                      ps_formula_vars_to_json(user.w_defaults));
  json_object_set_new(obj, "t_executed", json_real(user.t_executed));
  json_object_set_new(obj, "t_inactive_min", json_real(user.t_inactive_min));
  return obj;
}

json_t *ps_user_to_json_plain(const ps_user_t &user) {
  auto *obj = json_object();
  json_object_set_new(obj, "userid", json_integer((json_int_t)user.userid));
  if (user.score) {
    json_object_set_new(obj, "score", json_real(*user.score));
  }
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
  if (score_j) {
    if (!json_is_real(score_j)) {
      return ps_error(
          ps_field_type_mismatch_t{.field = "score", .expected = "double"});
    }
    res.score = json_number_value(score_j);
  }

  json_t *w_expr_j = json_object_get(obj, "w_expr");
  if (w_expr_j) {
    if (!json_is_string(w_expr_j)) {
      return ps_error(
          ps_field_type_mismatch_t{.field = "w_expr", .expected = "string"});
    }
    res.w_expr = json_string_value(w_expr_j);
  }

  json_t *w_defaults_j = json_object_get(obj, "w_defaults");
  if (w_defaults_j) {
    auto w_defaults_res = ps_formula_vars_from_json(w_defaults_j);
    if (!ps_is_ok(w_defaults_res)) {
      return ps_forward_error<ps_user_t>(w_defaults_res);
    }
    res.w_defaults = ps_unwrap(w_defaults_res);
  }

  json_t *t_executed_j = json_object_get(obj, "t_executed");
  if (t_executed_j) {
    if (!json_is_real(t_executed_j)) {
      return ps_error(ps_field_type_mismatch_t{.field = "t_executed",
                                               .expected = "double"});
    }
    res.t_executed = json_number_value(t_executed_j);
  }

  json_t *t_inactive_min_j = json_object_get(obj, "t_inactive_min");
  if (t_inactive_min_j) {
    if (!json_is_real(t_inactive_min_j)) {
      return ps_error(ps_field_type_mismatch_t{.field = "t_inactive_min",
                                               .expected = "double"});
    }
    res.t_inactive_min = json_number_value(t_inactive_min_j);
  }

  return res;
}
