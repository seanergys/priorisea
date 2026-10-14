// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/cli/endpoint/error.hpp"
#include "src/daemon/endpoint/error.hpp"
#include "src/database/error.hpp"
#include "src/lib/types.hpp"
#include "src/lib/util.hpp"
#include <flux/core.h>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

using ps_success_t = std::monostate;

struct ps_unexpected_err_t {
  std::string msg;
};

struct ps_permissions_err_t {
  std::string msg;
};

struct ps_flux_open_err_t {
  std::string strerror;
};
struct ps_flux_rpc_err_t {
  std::string strerror;
};

struct ps_field_missing_t {
  std::string field;
};
struct ps_field_type_mismatch_t {
  std::string field;
  std::string expected;
};

struct ps_string_parse_err_t {
  std::string msg;
};

struct ps_formula_eval_err_t {
  ps_formula_t formula;
  std::map<std::string, double> var_values;
  // 1-based character position tinyexpr reported for the parse failure.
  // <= 0 means tinyexpr gave no usable position.
  int error_pos = 0;
  // The expression actually handed to tinyexpr, after variable substitution.
  // error_pos indexes this string.
  std::string compiled_expr;
};

struct ps_flux_jobid_parse_err_t {
  std::string jobid;
  std::string strerror;
};

struct ps_access_denied_t {
  std::string key;
};

////

using ps_error_kind_t =
    std::variant<ps_unexpected_err_t, ps_permissions_err_t, ps_access_denied_t,
                 ps_field_missing_t, ps_field_type_mismatch_t,
                 ps_string_parse_err_t, ps_formula_eval_err_t,
                 ps_flux_open_err_t, ps_flux_rpc_err_t,
                 ps_flux_jobid_parse_err_t,

                 ps_daemon_err_t, ps_plugin_err_t,

                 ps_db_not_found_t, ps_db_already_exist_err_t, ps_db_open_err_t,
                 ps_db_version_mismatch_t, ps_db_query_err_t>;

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

template <typename T> std::string ps_exit_error(const ps_result_t<T> &r) {
  ps_exit_error(ps_unwrap_error(r));
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
 * Returns the error message for each error kind.
 *
 * These overloads are declared here (not merely defined in error.cpp) so that
 * call sites resolve to the specific message rather than implicitly widening
 * the argument to the ps_error_kind_t variant overload.
 */
std::string ps_error_message(const ps_unexpected_err_t &err);
std::string ps_error_message(const ps_access_denied_t &err);
std::string ps_error_message(const ps_field_missing_t &err);
std::string ps_error_message(const ps_field_type_mismatch_t &err);
std::string ps_error_message(const ps_string_parse_err_t &err);
std::string ps_error_message(const ps_formula_eval_err_t &err);
std::string ps_error_message(const ps_flux_open_err_t &err);
std::string ps_error_message(const ps_flux_rpc_err_t &err);
std::string ps_error_message(const ps_flux_jobid_parse_err_t &err);

/**
 * Returns the error message from an error kind.
 *
 * @param error The error kind.
 * @return The error message.
 */
std::string ps_error_message(const ps_error_kind_t &error);

/**
 * Returns the error message from an error result.
 *
 * @param e The error result.
 * @return The error message.
 */
std::string ps_error_message(const ps_error_t &e);

void ps_exit_error(const ps_error_t &e);
