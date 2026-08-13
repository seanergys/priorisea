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
struct ps_decay_duration_invalid_t {
  int value;
};
struct ps_decay_string_parse_err_t {
  std::string expr;
};

//// formula

struct ps_formula_expr_invalid_t {
  std::string expr;
};
struct ps_formula_invalid_name_t {
  std::string name;
};
struct ps_formula_eval_err_t {
  ps_formula_t formula;
  std::map<std::string, double> var_values;
};
struct ps_formula_name_missing_t {};
struct ps_formula_expr_missing_t {
  std::string name;
};
struct ps_formula_var_value_missing_t {
  std::string name;
};

//// timewin

struct ps_timewin_name_missing_t {};
struct ps_timewin_duration_missing_t {};
struct ps_timewin_cron_missing_t {};
struct ps_timewin_score_missing_t {};
struct ps_timewin_score_parse_err_t {};
struct ps_timewin_duration_parse_err_t {};
struct ps_cron_parse_err_t {
  std::string cron;
};
struct ps_timewin_neg_duration_t {
  int duration;
};

////

using ps_error_kind_t = std::variant<
    ps_str_empty_t, ps_parse_int_err_t, ps_parse_uid_err_t,
    ps_parse_double_err_t, ps_json_unpack_err_t, ps_json_pack_err_t,

    ps_flux_open_err_t, ps_flux_future_get_err_t, ps_flux_txn_create_err_t,
    ps_flux_kvs_commit_err_t, ps_flux_kvs_txn_create_err_t,
    ps_flux_kvs_txn_unlink_err_t, ps_flux_kvs_get_err_t,
    ps_flux_kvs_lookup_err_t, ps_flux_kvs_lookup_get_err_t,
    ps_flux_rpc_get_unpack_err_t, ps_flux_rpc_pack_err_t, ps_flux_rpc_err_t,
    ps_flux_jobid_parse_err_t, ps_flux_job_list_id_err_t,
    ps_flux_kvs_lookup_get_dir_err_t, ps_flux_kvsitr_create_err_t,
    ps_flux_kvs_txn_put_err_t,

    ps_decay_priority_missing_t, ps_decay_duration_invalid_t,
    ps_decay_string_parse_err_t,

    ps_formula_expr_invalid_t, ps_formula_invalid_name_t,
    ps_formula_name_missing_t, ps_formula_expr_missing_t,
    ps_formula_var_value_missing_t, ps_formula_eval_err_t,

    ps_timewin_name_missing_t, ps_timewin_duration_missing_t,
    ps_timewin_cron_missing_t, ps_timewin_score_missing_t,
    ps_timewin_score_parse_err_t, ps_timewin_duration_parse_err_t,
    ps_cron_parse_err_t, ps_timewin_neg_duration_t>;

struct ps_error_t {
  std::vector<std::string> context;
  ps_error_kind_t kind;
};

template <typename T> using ps_result_t = std::variant<T, ps_error_t>;

////

template <typename T> bool ps_is_ok(const ps_result_t<T> &r) {
  return std::holds_alternative<T>(r);
}

template <typename T> const T &ps_unwrap(const ps_result_t<T> &r) {
  if (!ps_is_ok(r)) {
    std::cerr << ps_error_message(r) << std::endl;
    std::abort();
  }
  return std::get<T>(r);
}

template <typename T>
const ps_error_t &ps_unwrap_error(const ps_result_t<T> &r) {
  if (ps_is_ok(r)) {
    std::cerr << "Expected error, got success: " << ps_type_name(r)
              << std::endl;
    std::abort();
  }
  return std::get<ps_error_t>(r);
}

template <typename To, typename From>
ps_result_t<To> ps_forward_error(const ps_result_t<From> &r,
                                 std::string ctx = "") {
  auto err = ps_unwrap_error(r);
  if (!ctx.empty()) {
    err.context.insert(err.context.begin(), std::move(ctx));
  }
  return err;
}

template <typename E>
ps_result_t<E> ps_add_context(const ps_result_t<E> &r, std::string ctx) {
  return ps_forward_error<E>(r, ctx);
}

template <typename T> std::string ps_error_message(const ps_result_t<T> &r) {
  auto err = ps_unwrap_error(r);
  return ps_error_message(err);
}

////

ps_error_t ps_error(ps_error_kind_t kind, std::string ctx = "");
std::string ps_error_message_kind(const ps_error_kind_t &error);
std::string ps_error_message(const ps_error_t &);
