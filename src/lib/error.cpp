// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "error.hpp"
#include "util.hpp"
#include <sstream>
#include <string>

ps_error_t ps_error(ps_error_kind_t kind, std::string ctx) {
  ps_error_t e{.kind = kind};
  if (!ctx.empty()) {
    e.context.push_back(std::move(ctx));
  }
  return e;
}

////

//// generic

std::string ps_error_message_kind(const ps_str_empty_t &) {
  return "string is empty";
}

std::string ps_error_message_kind(const ps_parse_int_err_t &err) {
  return "could not parse integer: " + err.int_str;
}

std::string ps_error_message_kind(const ps_parse_uid_err_t &err) {
  return "could not parse uid: " + err.uid_str;
}

std::string ps_error_message_kind(const ps_parse_double_err_t &err) {
  return "could not parse double: " + err.double_str;
}

std::string ps_error_message_kind(const ps_json_unpack_err_t &err) {
  return "could not unpack json: " + err.dumps;
}

std::string ps_error_message_kind(const ps_json_pack_err_t &) {
  return "json pack error";
}

std::string ps_error_message_kind(const ps_unimplemented_err_t &err) {
  return "feature unimplemented";
}

std::string ps_error_message_kind(const ps_no_file_err_t &err) {
  return "no such file: " + err.path;
}

//// flux

std::string ps_error_message_kind(const ps_flux_open_err_t &err) {
  return "flux open error: " + err.msg + ". Is flux running ?";
}

std::string ps_error_message_kind(const ps_flux_future_get_err_t &err) {
  return "flux future get error: " + err.msg;
}

std::string ps_error_message_kind(const ps_flux_txn_create_err_t &err) {
  return "flux transaction create error: " + err.msg;
}

std::string ps_error_message_kind(const ps_flux_kvs_commit_err_t &err) {
  return "flux kvs commit error: " + err.msg;
}

std::string ps_error_message_kind(const ps_flux_kvs_txn_create_err_t &err) {
  return "flux kvs transaction create error: " + err.msg;
}

std::string ps_error_message_kind(const ps_flux_kvs_txn_unlink_err_t &err) {
  return "flux kvs transaction unlink error for key '" + err.key +
         "': " + err.msg;
}

std::string ps_error_message_kind(const ps_flux_kvs_get_err_t &err) {
  return "flux kvs get error: " + err.msg;
}

std::string ps_error_message_kind(const ps_flux_kvs_lookup_err_t &err) {
  return "flux kvs lookup error for key '" + err.key + "': " + err.msg;
}

std::string ps_error_message_kind(const ps_flux_kvs_lookup_get_err_t &err) {
  return "flux kvs lookup get error for key '" + err.key +
         "'. Is the key valid?";
}

std::string ps_error_message_kind(const ps_flux_rpc_get_unpack_err_t &err) {
  return "flux rpc unpack error: " + err.msg;
}

std::string ps_error_message_kind(const ps_flux_rpc_pack_err_t &err) {
  return "flux rpc pack error: " + err.msg;
}

std::string ps_error_message_kind(const ps_flux_rpc_err_t &err) {
  return "flux rpc error: " + err.msg;
}

std::string ps_error_message_kind(const ps_flux_jobid_parse_err_t &err) {
  return "flux job id parse error for '" + err.jobid + "': " + err.msg +
         ". Is the job id valid?";
}

std::string ps_error_message_kind(const ps_flux_job_list_id_err_t &err) {
  return "flux job list error for id " + std::to_string(err.jobid) + ": " +
         err.msg + ". Does the job exist?";
}

std::string ps_error_message_kind(const ps_flux_kvs_lookup_get_dir_err_t &err) {
  return "flux kvs lookup get dir error for key '" + err.key + "': " + err.msg +
         ". Is the key valid?";
}

std::string ps_error_message_kind(const ps_flux_kvsitr_create_err_t &err) {
  return "flux kvs iterator create error for key '" + err.key +
         "': " + err.msg + ". Is the key valid?";
}

std::string ps_error_message_kind(const ps_flux_kvs_txn_put_err_t &err) {
  return "flux kvs transaction put error for key '" + err.key + "' value '" +
         err.value + "': " + err.msg;
}

//// decay

std::string ps_error_message_kind(const ps_decay_weight_missing_t &) {
  return "decay weight missing";
}

std::string ps_error_message_kind(const ps_decay_weight_invalid_t &) {
  return "decay weight invalid";
}

std::string ps_error_message_kind(const ps_decay_frame_duration_missing_t &) {
  return "decay frame duration missing";
}

std::string ps_error_message_kind(const ps_decay_frame_duration_invalid_t &) {
  return "decay frame duration invalid";
}

std::string ps_error_message_kind(const ps_decay_duration_invalid_t &err) {
  return "decay duration invalid: " + std::to_string(err.value);
}

std::string ps_error_message_kind(const ps_decay_string_parse_err_t &err) {
  return "decay string parse error: " + err.expr;
}

//// formula

std::string ps_error_message_kind(const ps_formula_expr_invalid_t &err) {
  std::string msg = "formula expression invalid";
  if (!err.name.empty()) {
    msg += " for '" + err.name + "'";
  }
  if (!err.expr.empty()) {
    msg += ": " + err.expr;
  }
  return msg;
}

std::string ps_error_message_kind(const ps_formula_eval_err_t &err) {
  std::string msg = "formula eval error. name: " + err.formula.name + "\n" +
                    err.compiled_expr;

  if (err.error_pos > 0) {
    size_t caret = static_cast<size_t>(err.error_pos) - 1;
    if (caret > err.compiled_expr.length())
      caret = err.compiled_expr.length();
    msg += "\n" + std::string(caret, ' ') + "^";
  }

  if (!err.formula.expr.empty() && err.formula.expr != err.compiled_expr)
    msg += "\n(original: " + err.formula.expr + ")";

  std::string var_err;
  for (const auto &[var_name, value] : err.var_values) {
    if (!var_err.empty())
      var_err += ", ";
    var_err += var_name + ": " + std::to_string(value);
  }
  if (!var_err.empty())
    msg += "\nvariables: " + var_err;

  return msg;
}

std::string ps_error_message_kind(const ps_formula_invalid_name_t &err) {
  return "formula invalid name: " + err.name;
}

std::string ps_error_message_kind(const ps_formula_name_missing_t &) {
  return "formula name missing";
}

std::string ps_error_message_kind(const ps_formula_name_invalid_t &) {
  return "formula name invalid";
}

std::string ps_error_message_kind(const ps_formula_expr_missing_t &err) {
  return "formula expression missing for: " + err.name;
}

std::string ps_error_message_kind(const ps_formula_defaults_missing_t &) {
  return "formula defaults missing";
}

std::string ps_error_message_kind(const ps_formula_var_value_missing_t &err) {
  return "formula variable value missing for: " + err.name;
}

//// timewin

std::string ps_error_message_kind(const ps_timewin_name_missing_t &) {
  return "time window name missing";
}

std::string ps_error_message_kind(const ps_timewin_name_invalid_t &) {
  return "time window name invalid";
}

std::string ps_error_message_kind(const ps_timewin_duration_missing_t &) {
  return "time window duration missing";
}

std::string ps_error_message_kind(const ps_timewin_duration_invalid_t &) {
  return "time window duration invalid";
}

std::string ps_error_message_kind(const ps_timewin_cron_missing_t &) {
  return "time window cron missing";
}

std::string ps_error_message_kind(const ps_timewin_cron_invalid_t &) {
  return "time window cron invalid";
}

std::string ps_error_message_kind(const ps_timewin_score_missing_t &) {
  return "time window score missing";
}

std::string ps_error_message_kind(const ps_timewin_score_invalid_t &) {
  return "time window score invalid";
}

std::string ps_error_message_kind(const ps_timewin_score_parse_err_t &) {
  return "time window score parse error";
}

std::string ps_error_message_kind(const ps_timewin_duration_parse_err_t &) {
  return "time window duration parse error";
}

std::string ps_error_message_kind(const ps_cron_parse_err_t &err) {
  return "cron parse error: " + err.cron;
}

std::string ps_error_message_kind(const ps_timewin_neg_duration_t &err) {
  return "time window negative duration: " + std::to_string(err.duration);
}

//// job

std::string ps_error_message_kind(const ps_job_jobid_missing_t &) {
  return "job id missing";
}

std::string ps_error_message_kind(const ps_job_t_inactive_missing_t &) {
  return "job t_inactive missing";
}

std::string ps_error_message_kind(const ps_job_score_missing_t &) {
  return "job score missing";
}

std::string ps_error_message_kind(const ps_job_jobid_invalid_t &) {
  return "job id invalid";
}

std::string ps_error_message_kind(const ps_job_t_inactive_invalid_t &) {
  return "job t_inactive invalid";
}

std::string ps_error_message_kind(const ps_job_score_invalid_t &) {
  return "job score invalid";
}

//// score map

std::string ps_error_message_kind(const ps_score_map_invalid_t &) {
  return "score map is not a JSON object";
}

std::string ps_error_message_kind(const ps_score_map_value_invalid_t &err) {
  return "score map value invalid for user '" + err.userid + "'";
}

std::string ps_error_message_kind(const ps_config_err_t &err) {
  return "config error at " + err.path + ": " + err.msg;
}

std::string ps_error_message_kind(const ps_seanerbus_url_missing_t &) {
  return "seanerbus url missing";
}

std::string ps_error_message_kind(const ps_seanerbus_url_invalid_t &) {
  return "seanerbus url invalid";
}

std::string ps_error_message_kind(const ps_seanerbus_resolve_err_t &err) {
  return "error when resolving seanerbus service: " + err.service;
}

std::string ps_error_message_kind(const ps_seanerbus_err_t &err) {
  return "seanerbus rpc error (" + err.service + "): " + err.msg;
}

std::string ps_error_message_kind(const ps_seanerbus_wrong_payload_err_t &err) {
  return "seanerbus wrong payload error (" + err.service + ")";
}

std::string ps_error_message_kind(const ps_seanerbus_decode_err_t &err) {
  return "seanerbus failed to decode error (" + err.service + "): " + err.msg;
}

std::string ps_error_message_kind(const ps_seanerbus_connection_err_t &err) {
  return "seanerbus (" + err.seanerbus + ") connection error: " + err.msg;
}

std::string ps_error_message_kind(const ps_invalid_uuid_err_t &err) {
  return "invalid uuid: " + err.uuid;
}

std::string ps_error_message_kind(const ps_invalid_rpc_ns_err_t &err) {
  return "invalid rpc_ns expression: " + err.expr;
}

std::string ps_error_message_kind(const ps_error_kind_t &error) {
  return "unknown error: " +
         std::visit([](const auto &e) { return ps_type_name(e); }, error);
}

////

std::string ps_error_message(const ps_error_t &e) {
  std::stringstream ss;
  for (const auto &context : e.context) {
    ss << context << "\n-> ";
  }

  ss << std::visit([](const auto &err) { return ps_error_message_kind(err); },
                   e.kind);
  return ss.str();
}
