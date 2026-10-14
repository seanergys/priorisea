// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/cli/endpoint/flux/formula.hpp"
#include "src/cli/endpoint/util.hpp"
#include "src/lib/error.hpp"
#include "src/lib/formula.hpp"
#include <cstring>
#include <flux/core.h>
#include <flux/core/future.h>
#include <jansson.h>
#include <string>

ps_result_t<ps_formula_t>
ps_cli_endpoint_flux_formula_get(const std::string &name) {
  auto h = flux_open(NULL, 0);
  if (!h) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }
  auto *f = flux_rpc_pack(h, "priorisea.daemon.formula.get", FLUX_NODEID_ANY, 0,
                          "{s:s}", "data", name.c_str());
  if (!f) {
    flux_close(h);
    return ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
  }

  json_t *formula_json;
  if (flux_rpc_get_unpack(f, "{s:o}", "data", &formula_json) < 0) {
    auto err = ps_cli_endpoint_unpack_err(f);
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(err);
  }
  auto formula = ps_formula_from_json(formula_json);
  flux_future_destroy(f);
  flux_close(h);
  return formula;
}

ps_result_t<std::vector<ps_formula_t>>
ps_cli_endpoint_flux_formula_list(const std::vector<std::string> &names) {
  auto h = flux_open(NULL, 0);
  if (!h) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }

  flux_future_t *f;
  if (names.empty()) {
    // No parameter: request the entire list
    // An empty object is sent
    f = flux_rpc_pack(h, "priorisea.daemon.formula.list", FLUX_NODEID_ANY, 0,
                      "{}");
  } else {
    json_t *data = json_array();
    for (const auto &name : names) {
      json_array_append_new(data, json_string(name.c_str()));
    }
    f = flux_rpc_pack(h, "priorisea.daemon.formula.list", FLUX_NODEID_ANY, 0,
                      "{s:O}", "data", data);
    json_decref(data);
  }
  if (!f) {
    flux_close(h);
    return ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
  }

  json_t *formulas_json;
  if (flux_rpc_get_unpack(f, "{s:o}", "data", &formulas_json) < 0) {
    auto err = ps_cli_endpoint_unpack_err(f);
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(err);
  }

  std::vector<ps_formula_t> formulas;
  size_t index;
  json_t *value;
  json_array_foreach(formulas_json, index, value) {
    auto formula_res = ps_formula_from_json(value);
    if (!ps_is_ok(formula_res)) {
      flux_future_destroy(f);
      flux_close(h);
      return ps_forward_error<std::vector<ps_formula_t>>(formula_res);
    }
    formulas.push_back(ps_unwrap(formula_res));
  }

  flux_future_destroy(f);
  flux_close(h);
  return formulas;
}

ps_result_t<ps_success_t>
ps_cli_endpoint_flux_formula_update(const ps_formula_t &formula) {
  auto h = flux_open(NULL, 0);
  if (!h) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }
  json_t *formula_json = ps_formula_to_json(formula);
  auto *f = flux_rpc_pack(h, "priorisea.daemon.formula.update", FLUX_NODEID_ANY,
                          0, "{s:O}", "data", formula_json);
  json_decref(formula_json);
  if (!f) {
    flux_close(h);
    return ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
  }
  if (flux_rpc_get(f, NULL) < 0) {
    auto err = std::string(flux_future_error_string(f));
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(ps_daemon_err_t{.err = err});
  }
  flux_future_destroy(f);
  flux_close(h);
  return ps_success_t{};
}

ps_result_t<ps_success_t>
ps_cli_endpoint_flux_formula_reset(const std::vector<std::string> &names) {
  auto h = flux_open(NULL, 0);
  if (!h) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }

  json_t *data = json_array();
  for (const auto &name : names) {
    json_array_append_new(data, json_string(name.c_str()));
  }

  flux_future_t *f = flux_rpc_pack(h, "priorisea.daemon.formula.reset",
                                   FLUX_NODEID_ANY, 0, "{s:O}", "data", data);
  json_decref(data);
  if (!f) {
    flux_close(h);
    return ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
  }

  if (flux_rpc_get(f, NULL) < 0) {
    auto err = std::string(flux_future_error_string(f));
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(ps_daemon_err_t{.err = err});
  }
  flux_future_destroy(f);
  flux_close(h);
  return ps_success_t{};
}
