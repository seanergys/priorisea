// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/daemon/endpoint/flux/callback/formula.hpp"
#include "src/database/formula.hpp"
#include "src/database/sqlite/formula.hpp"
#include "src/lib/error.hpp"
#include "src/lib/formula.hpp"
#include <errno.h>
#include <flux/core.h>
#include <flux/core/response.h>
#include <jansson.h>
#include <string>

int ps_daemon_endpoint_flux_formula_get(
    const ps_daemon_endpoint_flux_callback_args_t &args) {
  // Retrieve the formula name from the payload
  const char *name;
  if (flux_request_unpack(args.msg, NULL, "{s:s}", "data", &name) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }

  // Get the formula from the database
  auto get_res = ps_db_sqlite_formula_get(args.db, name);
  if (!ps_is_ok(get_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(get_res).c_str());
  }
  auto formula = ps_unwrap(get_res);

  // Return the single formula
  return flux_respond_pack(args.h, args.msg, "{s:o}", "data",
                           ps_formula_to_json(formula));
}

int ps_daemon_endpoint_flux_formula_list(
    const ps_daemon_endpoint_flux_callback_args_t &args) {
  // Retrieve the optional array of formula names from the payload
  json_t *data_json = nullptr;
  if (flux_request_unpack(args.msg, NULL, "{s?o}", "data", &data_json) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }

  std::vector<ps_formula_t> formulas;
  if (data_json == nullptr) {
    // No parameter: return the entire list
    auto list_res = ps_db_sqlite_formulas_list(args.db);
    if (!ps_is_ok(list_res)) {
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(list_res).c_str());
    }
    formulas = ps_unwrap(list_res);
  } else {
    if (!json_is_array(data_json)) {
      auto err = ps_field_type_mismatch_t{.field = "data", .expected = "array"};
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(err).c_str());
    }

    // Build the list of formula names
    std::vector<std::string> names;
    size_t index;
    json_t *value;
    json_array_foreach(data_json, index, value) {
      if (!json_is_string(value)) {
        auto err = ps_field_type_mismatch_t{
            .field = "data[" + std::to_string(index) + "]",
            .expected = "string"};
        return flux_respond_error(args.h, args.msg, 0,
                                  ps_error_message(err).c_str());
      }
      names.push_back(json_string_value(value));
    }

    // Get the formulas from the database
    auto formulas_res = ps_db_sqlite_formulas_get(args.db, names);
    if (!ps_is_ok(formulas_res)) {
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(formulas_res).c_str());
    }
    auto formulas_map = ps_unwrap(formulas_res);

    // Build the list of formulas, preserving the request order
    for (const auto &name : names) {
      // The formula is guaranteed to be in the map
      auto &formula_res = formulas_map[name];
      if (!ps_is_ok(formula_res)) {
        return flux_respond_error(args.h, args.msg, 0,
                                  ps_error_message(formula_res).c_str());
      }
      formulas.push_back(ps_unwrap(formula_res));
    }
  }

  // Return as an array of formulas
  json_t *formulas_json = json_array();
  for (const auto &formula : formulas) {
    json_array_append(formulas_json, ps_formula_to_json(formula));
  }
  return flux_respond_pack(args.h, args.msg, "{s:o}", "data", formulas_json);
}

int ps_daemon_endpoint_flux_formula_update(
    const ps_daemon_endpoint_flux_callback_args_t &args) {
  // Retrieve the formula from the payload
  json_t *formula_json;
  if (flux_request_unpack(args.msg, NULL, "{s:o}", "data", &formula_json) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }
  auto formula_res = ps_formula_from_json(formula_json);
  if (!ps_is_ok(formula_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(formula_res).c_str());
  }
  auto formula = ps_unwrap(formula_res);

  // Validate the formula
  auto validate_res = ps_validate_formula(formula);
  if (!ps_is_ok(validate_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(validate_res).c_str());
  }

  // Update the formula in the database (fails if it does not exist)
  auto update_res = ps_db_sqlite_formula_update(args.db, formula);
  if (!ps_is_ok(update_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(update_res).c_str());
  }
  return flux_respond(args.h, args.msg, NULL);
}

int ps_daemon_endpoint_flux_formula_reset(
    const ps_daemon_endpoint_flux_callback_args_t &args) {
  // Retrieve the array of formula names from the payload
  json_t *data_json;
  if (flux_request_unpack(args.msg, NULL, "{s:o}", "data", &data_json) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }

  if (!json_is_array(data_json)) {
    auto err = ps_field_type_mismatch_t{.field = "data", .expected = "array"};
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }

  // Build the list of formula names
  std::vector<std::string> names;
  size_t index;
  json_t *value;
  json_array_foreach(data_json, index, value) {
    if (!json_is_string(value)) {
      auto err = ps_field_type_mismatch_t{
          .field = "data[" + std::to_string(index) + "]", .expected = "string"};
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(err).c_str());
    }
    names.push_back(json_string_value(value));
  }

  if (names.size() == 0) {
    return flux_respond(args.h, args.msg, NULL);
  }

  auto reset_res = ps_db_formulas_reset(args.db, names);
  if (!ps_is_ok(reset_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(reset_res).c_str());
  }

  return flux_respond(args.h, args.msg, NULL);
}
