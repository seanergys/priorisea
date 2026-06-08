// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "error.hpp"
#include "util.hpp"
#include <sstream>

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

std::string ps_error_message_kind(const ps_parse_double_err_t &err) {
  return "could not parse double: " + err.double_str;
}

std::string ps_error_message_kind(const ps_json_unpack_err_t &) {
  return "json unpack error";
}

std::string ps_error_message_kind(const ps_json_pack_err_t &) {
  return "json pack error";
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

std::string ps_error_message_kind(const ps_flux_job_id_parse_err_t &err) {
  return "flux job id parse error for '" + err.jobid + "': " + err.msg +
         ". Is the job id valid?";
}

std::string ps_error_message_kind(const ps_flux_job_list_id_err_t &err) {
  return "flux job list error for id " + std::to_string(err.id) + ": " +
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

std::string ps_error_message_kind(const ps_decay_priority_missing_t &) {
  return "decay priority missing";
}

std::string ps_error_message_kind(const ps_decay_duration_invalid_t &err) {
  return "decay duration invalid: " + std::to_string(err.value);
}

//// formula

std::string ps_error_message_kind(const ps_formula_expr_invalid_t &err) {
  return "formula expression invalid: " + err.expr;
}

std::string ps_error_message_kind(const ps_formula_invalid_name_t &err) {
  return "formula invalid name: " + err.name;
}

std::string ps_error_message_kind(const ps_formula_name_missing_t &) {
  return "formula name missing";
}

std::string ps_error_message_kind(const ps_formula_expr_missing_t &err) {
  return "formula expression missing for: " + err.name;
}

std::string ps_error_message_kind(const ps_formula_var_value_missing_t &err) {
  return "formula variable value missing for: " + err.name;
}

//// score

std::string ps_error_message_kind(const ps_formula_job_score_invalid_t &) {
  return "formula job score invalid";
}

//// timewin

std::string ps_error_message_kind(const ps_timewin_name_missing_t &) {
  return "time window name missing";
}

std::string ps_error_message_kind(const ps_timewin_duration_missing_t &) {
  return "time window duration missing";
}

std::string ps_error_message_kind(const ps_timewin_cron_missing_t &) {
  return "time window cron missing";
}

std::string ps_error_message_kind(const ps_timewin_priority_missing_t &) {
  return "time window priority missing";
}

std::string ps_error_message_kind(const ps_timewin_priority_parse_err_t &) {
  return "time window priority parse error";
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

std::string ps_error_message_kind(const ps_error_kind_t &error) {
  return "unknown error: " + ps_type_name(error);
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
