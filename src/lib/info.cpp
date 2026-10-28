#include "src/lib/info.hpp"
#include "src/lib/error.hpp"

json_t *ps_db_info_to_json(const ps_db_info_t &info) {
  auto *obj = json_object();
  json_object_set_new(obj, "t_inactive", json_real(info.t_inactive));
  json_object_set_new(obj, "user_period", json_real(info.user_period));
  return obj;
}

ps_result_t<ps_db_info_t> ps_db_info_from_json(json_t *obj) {
  ps_db_info_t res;
  json_t *t_inactive_j = json_object_get(obj, "t_inactive");
  if (!t_inactive_j) {
    return ps_error(ps_field_missing_t{.field = "t_inactive"});
  }
  if (!json_is_real(t_inactive_j)) {
    return ps_error(
        ps_field_type_mismatch_t{.field = "t_inactive", .expected = "double"});
  }
  res.t_inactive = json_number_value(t_inactive_j);

  json_t *user_period_j = json_object_get(obj, "user_period");
  if (!user_period_j) {
    return ps_error(ps_field_missing_t{.field = "user_period"});
  }
  if (!json_is_real(user_period_j)) {
    return ps_error(
        ps_field_type_mismatch_t{.field = "user_period", .expected = "double"});
  }
  res.user_period = json_number_value(user_period_j);
  return res;
}
