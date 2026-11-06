#include "src/cli/endpoint/flux/timewin.hpp"
#include "src/cli/endpoint/util.hpp"
#include "src/lib/error.hpp"
#include "src/lib/timewin.hpp"
#include <flux/core.h>
#include <jansson.h>
#include <string>

ps_result_t<ps_timewin_t>
ps_cli_endpoint_flux_timewin_get(const std::string &name) {
  return ps_cli_endpoint_flux_rpc<ps_timewin_t>(
      [&name](flux_t *h) {
        return flux_rpc_pack(h, "priorisea.daemon.timewin.get", FLUX_NODEID_ANY,
                             0, "{s:s}", "data", name.c_str());
      },
      [](flux_future_t *f) -> ps_result_t<ps_timewin_t> {
        json_t *timewin_json;
        if (flux_rpc_get_unpack(f, "{s:o}", "data", &timewin_json) < 0) {
          return ps_error(ps_cli_endpoint_unpack_err(f));
        }
        return ps_timewin_from_json(timewin_json);
      });
}

ps_result_t<std::vector<ps_timewin_t>>
ps_cli_endpoint_flux_timewin_list(const std::vector<std::string> &names) {
  return ps_cli_endpoint_flux_rpc<std::vector<ps_timewin_t>>(
      [&](flux_t *h) -> flux_future_t * {
        if (names.empty()) {
          // No parameter: request the entire list
          // An empty object is sent
          return flux_rpc_pack(h, "priorisea.daemon.timewin.list",
                               FLUX_NODEID_ANY, 0, "{}");
        }
        json_t *data = json_array();
        for (const auto &name : names) {
          json_array_append_new(data, json_string(name.c_str()));
        }
        auto *f = flux_rpc_pack(h, "priorisea.daemon.timewin.list",
                                FLUX_NODEID_ANY, 0, "{s:O}", "data", data);
        json_decref(data);
        return f;
      },
      [&](flux_future_t *f) -> ps_result_t<std::vector<ps_timewin_t>> {
        json_t *timewins_json;
        if (flux_rpc_get_unpack(f, "{s:o}", "data", &timewins_json) < 0) {
          return ps_error(ps_cli_endpoint_unpack_err(f));
        }
        std::vector<ps_timewin_t> timewins;
        size_t index;
        json_t *value;
        json_array_foreach(timewins_json, index, value) {
          auto timewin_res = ps_timewin_from_json(value);
          if (!ps_is_ok(timewin_res)) {
            return ps_forward_error<std::vector<ps_timewin_t>>(timewin_res);
          }
          timewins.push_back(ps_unwrap(timewin_res));
        }
        return timewins;
      });
}

ps_result_t<ps_success_t>
ps_cli_endpoint_flux_timewin_update(const ps_timewin_t &timewin) {
  return ps_cli_endpoint_flux_rpc<ps_success_t>(
      [&](flux_t *h) {
        json_t *timewin_json = ps_timewin_to_json(timewin);
        auto *f =
            flux_rpc_pack(h, "priorisea.daemon.timewin.update", FLUX_NODEID_ANY,
                          0, "{s:O}", "data", timewin_json);
        json_decref(timewin_json);
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
ps_cli_endpoint_flux_timewin_create(const ps_timewin_t &timewin) {
  return ps_cli_endpoint_flux_rpc<ps_success_t>(
      [&](flux_t *h) {
        json_t *timewin_json = ps_timewin_to_json(timewin);
        auto *f =
            flux_rpc_pack(h, "priorisea.daemon.timewin.create", FLUX_NODEID_ANY,
                          0, "{s:O}", "data", timewin_json);
        json_decref(timewin_json);
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
ps_cli_endpoint_flux_timewin_delete(const std::vector<std::string> &names) {
  return ps_cli_endpoint_flux_rpc<ps_success_t>(
      [&](flux_t *h) {
        json_t *data = json_array();
        for (const auto &name : names) {
          json_array_append_new(data, json_string(name.c_str()));
        }
        auto *f = flux_rpc_pack(h, "priorisea.daemon.timewin.delete",
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
