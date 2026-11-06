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
  return ps_cli_endpoint_flux_rpc<ps_formula_t>(
      [&](flux_t *h) {
        return flux_rpc_pack(h, "priorisea.daemon.formula.get", FLUX_NODEID_ANY,
                             0, "{s:s}", "data", name.c_str());
      },
      [&](flux_future_t *f) -> ps_result_t<ps_formula_t> {
        json_t *formula_json;
        if (flux_rpc_get_unpack(f, "{s:o}", "data", &formula_json) < 0) {
          return ps_error(ps_cli_endpoint_unpack_err(f));
        }
        return ps_formula_from_json(formula_json);
      });
}

ps_result_t<std::vector<ps_formula_t>>
ps_cli_endpoint_flux_formula_list(const std::vector<std::string> &names) {
  return ps_cli_endpoint_flux_rpc<std::vector<ps_formula_t>>(
      [&](flux_t *h) -> flux_future_t * {
        if (names.empty()) {
          // No parameter: request the entire list
          // An empty object is sent
          return flux_rpc_pack(h, "priorisea.daemon.formula.list",
                               FLUX_NODEID_ANY, 0, "{}");
        }
        json_t *data = json_array();
        for (const auto &name : names) {
          json_array_append_new(data, json_string(name.c_str()));
        }
        auto *f = flux_rpc_pack(h, "priorisea.daemon.formula.list",
                                FLUX_NODEID_ANY, 0, "{s:O}", "data", data);
        json_decref(data);
        return f;
      },
      [&](flux_future_t *f) -> ps_result_t<std::vector<ps_formula_t>> {
        json_t *formulas_json;
        if (flux_rpc_get_unpack(f, "{s:o}", "data", &formulas_json) < 0) {
          return ps_error(ps_cli_endpoint_unpack_err(f));
        }
        std::vector<ps_formula_t> formulas;
        size_t index;
        json_t *value;
        json_array_foreach(formulas_json, index, value) {
          auto formula_res = ps_formula_from_json(value);
          if (!ps_is_ok(formula_res)) {
            return ps_forward_error<std::vector<ps_formula_t>>(formula_res);
          }
          formulas.push_back(ps_unwrap(formula_res));
        }
        return formulas;
      });
}

ps_result_t<ps_success_t>
ps_cli_endpoint_flux_formula_update(const ps_formula_t &formula) {
  return ps_cli_endpoint_flux_rpc<ps_success_t>(
      [&](flux_t *h) {
        json_t *formula_json = ps_formula_to_json(formula);
        auto *f =
            flux_rpc_pack(h, "priorisea.daemon.formula.update", FLUX_NODEID_ANY,
                          0, "{s:O}", "data", formula_json);
        json_decref(formula_json);
        return f;
      },
      [&](flux_future_t *f) -> ps_result_t<ps_success_t> {
        if (flux_rpc_get(f, NULL) < 0) {
          return ps_error(
              ps_daemon_err_t{.err = std::string(flux_future_error_string(f))});
        }
        return ps_success_t{};
      });
}

ps_result_t<ps_success_t>
ps_cli_endpoint_flux_formula_reset(const std::vector<std::string> &names) {
  return ps_cli_endpoint_flux_rpc<ps_success_t>(
      [&](flux_t *h) {
        json_t *data = json_array();
        for (const auto &name : names) {
          json_array_append_new(data, json_string(name.c_str()));
        }
        auto *f = flux_rpc_pack(h, "priorisea.daemon.formula.reset",
                                FLUX_NODEID_ANY, 0, "{s:O}", "data", data);
        json_decref(data);
        return f;
      },
      [&](flux_future_t *f) -> ps_result_t<ps_success_t> {
        if (flux_rpc_get(f, NULL) < 0) {
          return ps_error(
              ps_daemon_err_t{.err = std::string(flux_future_error_string(f))});
        }
        return ps_success_t{};
      });
}
