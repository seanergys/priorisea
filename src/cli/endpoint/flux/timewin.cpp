// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/cli/endpoint/flux/timewin.hpp"
#include "src/cli/endpoint/util.hpp"
#include "src/lib/error.hpp"
#include "src/lib/timewin.hpp"
#include <flux/core.h>
#include <jansson.h>
#include <string>

ps_result_t<ps_timewin_t>
ps_cli_endpoint_flux_timewin_get(const std::string &name) {
  auto h = flux_open(NULL, 0);
  if (!h) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }
  auto *f = flux_rpc_pack(h, "priorisea.daemon.timewin.get", FLUX_NODEID_ANY, 0,
                          "{s:s}", "data", name.c_str());
  if (!f) {
    flux_close(h);
    return ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
  }

  json_t *timewin_json;
  if (flux_rpc_get_unpack(f, "{s:o}", "data", &timewin_json) < 0) {
    auto err = ps_cli_endpoint_unpack_err(f);
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(err);
  }
  auto timewin = ps_timewin_from_json(timewin_json);
  flux_future_destroy(f);
  flux_close(h);
  return timewin;
}

ps_result_t<std::vector<ps_timewin_t>>
ps_cli_endpoint_flux_timewin_list(const std::vector<std::string> &names) {
  auto h = flux_open(NULL, 0);
  if (!h) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }

  flux_future_t *f;
  if (names.empty()) {
    // No parameter: request the entire list
    // An empty object is sent
    f = flux_rpc_pack(h, "priorisea.daemon.timewin.list", FLUX_NODEID_ANY, 0,
                      "{}");
  } else {
    json_t *data = json_array();
    for (const auto &name : names) {
      json_array_append_new(data, json_string(name.c_str()));
    }
    f = flux_rpc_pack(h, "priorisea.daemon.timewin.list", FLUX_NODEID_ANY, 0,
                      "{s:O}", "data", data);
    json_decref(data);
  }
  if (!f) {
    flux_close(h);
    return ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
  }

  json_t *timewins_json;
  if (flux_rpc_get_unpack(f, "{s:o}", "data", &timewins_json) < 0) {
    auto err = ps_cli_endpoint_unpack_err(f);
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(err);
  }

  std::vector<ps_timewin_t> timewins;
  size_t index;
  json_t *value;
  json_array_foreach(timewins_json, index, value) {
    auto timewin_res = ps_timewin_from_json(value);
    if (!ps_is_ok(timewin_res)) {
      flux_future_destroy(f);
      flux_close(h);
      return ps_forward_error<std::vector<ps_timewin_t>>(timewin_res);
    }
    timewins.push_back(ps_unwrap(timewin_res));
  }

  flux_future_destroy(f);
  flux_close(h);
  return timewins;
}

ps_result_t<ps_success_t>
ps_cli_endpoint_flux_timewin_update(const ps_timewin_t &timewin) {
  auto h = flux_open(NULL, 0);
  if (!h) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }
  json_t *timewin_json = ps_timewin_to_json(timewin);
  auto *f = flux_rpc_pack(h, "priorisea.daemon.timewin.update", FLUX_NODEID_ANY,
                          0, "{s:O}", "data", timewin_json);
  json_decref(timewin_json);
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
ps_cli_endpoint_flux_timewin_create(const ps_timewin_t &timewin) {
  auto h = flux_open(NULL, 0);
  if (!h) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }
  json_t *timewin_json = ps_timewin_to_json(timewin);
  auto *f = flux_rpc_pack(h, "priorisea.daemon.timewin.create", FLUX_NODEID_ANY,
                          0, "{s:O}", "data", timewin_json);
  json_decref(timewin_json);
  if (!f) {
    flux_close(h);
    return ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
  }
  if (flux_rpc_get(f, NULL) < 0) {
    auto err = std::string{flux_future_error_string(f)};
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(ps_daemon_err_t{.err = err});
  }
  flux_future_destroy(f);
  flux_close(h);
  return ps_success_t{};
}

ps_result_t<ps_success_t>
ps_cli_endpoint_flux_timewin_delete(const std::vector<std::string> &names) {
  auto h = flux_open(NULL, 0);
  if (!h) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }
  json_t *data = json_array();
  for (const auto &name : names) {
    json_array_append_new(data, json_string(name.c_str()));
  }
  auto *f = flux_rpc_pack(h, "priorisea.daemon.timewin.delete", FLUX_NODEID_ANY,
                          0, "{s:O}", "data", data);
  json_decref(data);
  if (!f) {
    flux_close(h);
    return ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
  }
  if (flux_rpc_get(f, NULL) < 0) {
    auto err = std::string{flux_future_error_string(f)};
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(ps_daemon_err_t{.err = err});
  }
  flux_future_destroy(f);
  flux_close(h);
  return ps_success_t{};
}
