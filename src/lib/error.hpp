// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "types.hpp"
#include "util.hpp"
#include <flux/core.h>
#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

//// generic

using ps_success_t = std::monostate;
struct ps_str_empty_t {};
struct ps_parse_int_err_t {
  std::string int_str;
};
struct ps_parse_uid_err_t {
  std::string uid_str;
};
struct ps_parse_double_err_t {
  std::string double_str;
};
struct ps_json_unpack_err_t {
  std::string dumps;
};
struct ps_json_pack_err_t {};
struct ps_unimplemented_err_t {};
struct ps_no_file_err_t {
  std::string path;
};

//// flux

struct ps_flux_open_err_t {
  std::string msg;
};
struct ps_flux_txn_create_err_t {
  std::string msg;
};
struct ps_flux_future_get_err_t {
  std::string msg;
};
struct ps_flux_kvs_commit_err_t {
  std::string msg;
};
struct ps_flux_kvs_txn_create_err_t {
  std::string msg;
};
struct ps_flux_kvs_txn_unlink_err_t {
  std::string key;
  std::string msg;
};
struct ps_flux_kvs_get_err_t {
  std::string msg;
};
struct ps_flux_kvs_lookup_err_t {
  std::string key;
  std::string msg;
};
struct ps_flux_kvs_lookup_get_err_t {
  std::string key;
};
struct ps_flux_rpc_get_unpack_err_t {
  std::string msg;
};
struct ps_flux_rpc_err_t {
  std::string msg;
};
struct ps_flux_rpc_pack_err_t {
  std::string msg;
};
struct ps_flux_jobid_parse_err_t {
  std::string jobid;
  std::string msg;
};
struct ps_flux_job_list_id_err_t {
  flux_jobid_t jobid;
  std::string msg;
};
struct ps_flux_kvs_lookup_get_dir_err_t {
  std::string key;
  std::string msg;
};
struct ps_flux_kvsitr_create_err_t {
  std::string key;
  std::string msg;
};
struct ps_flux_kvs_txn_put_err_t {
  std::string key;
  std::string value;
  std::string msg;
};

//// decay

struct ps_decay_priority_missing_t {};
struct ps_decay_priority_invalid_t {};
struct ps_decay_frame_duration_missing_t {};
struct ps_decay_frame_duration_invalid_t {};
struct ps_decay_duration_invalid_t {
  int value;
};
struct ps_decay_string_parse_err_t {
  std::string expr;
};

//// formula

struct ps_formula_expr_invalid_t {
  std::string expr;
  std::string name;
};
struct ps_formula_invalid_name_t {
  std::string name;
};
struct ps_formula_eval_err_t {
  ps_formula_t formula;
  std::map<std::string, double> var_values;
  // 1-based character position tinyexpr reported for the parse failure.
  // <= 0 means tinyexpr gave no usable position.
  int error_pos = 0;
  // The expression actually handed to tinyexpr, after variable substitution.
  // error_pos indexes this string, not formula.expr.
  std::string compiled_expr;
};
struct ps_formula_name_missing_t {};
struct ps_formula_name_invalid_t {};
struct ps_formula_expr_missing_t {
  std::string name;
};
struct ps_formula_defaults_missing_t {};
struct ps_formula_var_value_missing_t {
  std::string name;
};

//// timewin

struct ps_timewin_name_missing_t {};
struct ps_timewin_name_invalid_t {};
struct ps_timewin_duration_missing_t {};
struct ps_timewin_duration_invalid_t {};
struct ps_timewin_cron_missing_t {};
struct ps_timewin_cron_invalid_t {};
struct ps_timewin_score_missing_t {};
struct ps_timewin_score_invalid_t {};
struct ps_timewin_score_parse_err_t {};
struct ps_timewin_duration_parse_err_t {};
struct ps_cron_parse_err_t {
  std::string cron;
};
struct ps_timewin_neg_duration_t {
  int duration;
};

//// job

struct ps_job_jobid_missing_t {};
struct ps_job_jobid_invalid_t {};
struct ps_job_t_inactive_missing_t {};
struct ps_job_t_inactive_invalid_t {};
struct ps_job_score_missing_t {};
struct ps_job_score_invalid_t {};

//// score map

struct ps_score_map_invalid_t {};
struct ps_score_map_value_invalid_t {
  std::string userid;
};

//// config

struct ps_config_err_t {
  std::string path;
  std::string msg;
};

//// seanerbus

struct ps_seanerbus_ip_missing_t {};
struct ps_seanerbus_ip_invalid_t {};
struct ps_seanerbus_port_missing_t {};
struct ps_seanerbus_port_invalid_t {};
struct ps_seanerbus_resolve_err_t {
  std::string service;
};
struct ps_seanerbus_err_t {
  std::string service;
  std::string msg;
};
struct ps_seanerbus_wrong_payload_err_t {
  std::string service;
};
struct ps_seanerbus_decode_err_t {
  std::string service;
  std::string msg;
};
struct ps_seanerbus_connection_err_t {
  ps_seanerbus_t seanerbus;
  std::string msg;
};
struct ps_invalid_uuid_err_t {
  std::string uuid;
};
struct ps_invalid_rpc_ns_err_t {
  std::string expr;
};

////

using ps_error_kind_t = std::variant<
    ps_str_empty_t, ps_parse_int_err_t, ps_parse_uid_err_t,
    ps_parse_double_err_t, ps_json_unpack_err_t, ps_json_pack_err_t,
    ps_unimplemented_err_t, ps_no_file_err_t,

    ps_flux_open_err_t, ps_flux_future_get_err_t, ps_flux_txn_create_err_t,
    ps_flux_kvs_commit_err_t, ps_flux_kvs_txn_create_err_t,
    ps_flux_kvs_txn_unlink_err_t, ps_flux_kvs_get_err_t,
    ps_flux_kvs_lookup_err_t, ps_flux_kvs_lookup_get_err_t,
    ps_flux_rpc_get_unpack_err_t, ps_flux_rpc_pack_err_t, ps_flux_rpc_err_t,
    ps_flux_jobid_parse_err_t, ps_flux_job_list_id_err_t,
    ps_flux_kvs_lookup_get_dir_err_t, ps_flux_kvsitr_create_err_t,
    ps_flux_kvs_txn_put_err_t,

    ps_decay_priority_missing_t, ps_decay_priority_invalid_t,
    ps_decay_frame_duration_missing_t, ps_decay_frame_duration_invalid_t,
    ps_decay_duration_invalid_t, ps_decay_string_parse_err_t,

    ps_formula_expr_invalid_t, ps_formula_invalid_name_t,
    ps_formula_name_missing_t, ps_formula_name_invalid_t,
    ps_formula_expr_missing_t, ps_formula_defaults_missing_t,
    ps_formula_var_value_missing_t, ps_formula_eval_err_t,

    ps_timewin_name_missing_t, ps_timewin_name_invalid_t,
    ps_timewin_duration_missing_t, ps_timewin_duration_invalid_t,
    ps_timewin_cron_missing_t, ps_timewin_cron_invalid_t,
    ps_timewin_score_missing_t, ps_timewin_score_invalid_t,
    ps_timewin_score_parse_err_t, ps_timewin_duration_parse_err_t,
    ps_cron_parse_err_t, ps_timewin_neg_duration_t,

    ps_job_jobid_missing_t, ps_job_jobid_invalid_t, ps_job_t_inactive_missing_t,
    ps_job_t_inactive_invalid_t, ps_job_score_missing_t, ps_job_score_invalid_t,

    ps_score_map_invalid_t, ps_score_map_value_invalid_t,

    ps_config_err_t,

    ps_seanerbus_ip_missing_t, ps_seanerbus_ip_invalid_t,
    ps_seanerbus_port_missing_t, ps_seanerbus_port_invalid_t,
    ps_seanerbus_resolve_err_t, ps_seanerbus_err_t,
    ps_seanerbus_wrong_payload_err_t, ps_seanerbus_decode_err_t,
    ps_seanerbus_connection_err_t, ps_invalid_uuid_err_t,
    ps_invalid_rpc_ns_err_t>;

struct ps_error_t {
  std::vector<std::string> context;
  ps_error_kind_t kind;
};

template <typename T> using ps_result_t = std::variant<T, ps_error_t>;

/**
 * Checks if the result is an error.
 *
 * @param r The result to check.
 * @return True if the result is an error, false otherwise.
 */
template <typename T> bool ps_is_ok(const ps_result_t<T> &r) {
  return std::holds_alternative<T>(r);
}

/**
 * Checks if the result is an error of a specific type.
 *
 * @param r The result to check.
 * @return True if the result is an error of the specified type, false
 * otherwise.
 */
template <typename E, typename T>
bool ps_is_error_type(const ps_result_t<T> &r) {
  if (ps_is_ok(r)) {
    return false;
  }
  return std::holds_alternative<E>(std::get<ps_error_t>(r).kind);
}

/**
 * Unwraps the value from a successful result.
 *
 * @param r The result to unwrap.
 * @return The value from the result.
 */
template <typename T> const T &ps_unwrap(const ps_result_t<T> &r) {
  if (!ps_is_ok(r)) {
    std::cerr << ps_error_message(r) << std::endl;
    std::abort();
  }
  return std::get<T>(r);
}

/**
 * Unwraps the error from a failed result.
 *
 * @param r The result to unwrap.
 * @return The error from the result.
 */
template <typename T>
const ps_error_t &ps_unwrap_error(const ps_result_t<T> &r) {
  if (ps_is_ok(r)) {
    std::cerr << "Expected error, got success: " << ps_type_name(r)
              << std::endl;
    std::abort();
  }
  return std::get<ps_error_t>(r);
}

/**
 * Forwards the error from a result to a new result of a different type.
 *
 * @param r The result to forward.
 * @param ctx The context to add to the error.
 * @return The forwarded error.
 */
template <typename To, typename From>
ps_result_t<To> ps_forward_error(const ps_result_t<From> &r,
                                 std::string ctx = "") {
  auto err = ps_unwrap_error(r);
  if (!ctx.empty()) {
    err.context.insert(err.context.begin(), std::move(ctx));
  }
  return err;
}

/**
 * Adds context to an error result.
 *
 * @param r The result to add context to.
 * @param ctx The context to add.
 * @return The result with the added context.
 */
template <typename E>
ps_result_t<E> ps_add_context(const ps_result_t<E> &r, std::string ctx) {
  return ps_forward_error<E>(r, ctx);
}

/**
 * Returns the error message from a result.
 *
 * @param r The result to get the error message from.
 * @return The error message.
 */
template <typename T> std::string ps_error_message(const ps_result_t<T> &r) {
  auto err = ps_unwrap_error(r);
  return ps_error_message(err);
}

/**
 * Creates an error result from an error kind and optional context.
 *
 * @param kind The error kind.
 * @param ctx The context to add.
 * @return The error result.
 */
ps_error_t ps_error(ps_error_kind_t kind, std::string ctx = "");

/**
 * Returns the error message from an error kind.
 *
 * @param error The error kind.
 * @return The error message.
 */
std::string ps_error_message_kind(const ps_error_kind_t &error);

/**
 * Returns the error message from an error result.
 *
 * @param error The error result.
 * @return The error message.
 */
std::string ps_error_message(const ps_error_t &);
