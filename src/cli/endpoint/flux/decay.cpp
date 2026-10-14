// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/lib/decay.hpp"
#include "src/cli/endpoint/error.hpp"
#include "src/cli/endpoint/util.hpp"
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <flux/core.h>
#include <jansson.h>

ps_result_t<ps_decay_t> ps_cli_endpoint_flux_decay_get() {
  auto h = flux_open(NULL, 0);
  if (!h) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }
  auto f = flux_rpc(h, "priorisea.daemon.decay.get", NULL, FLUX_NODEID_ANY, 0);
  if (!f) {
    flux_close(h);
    return ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
  }

  json_t *decay_json;
  if (flux_rpc_get_unpack(f, "{s:o}", "data", &decay_json) < 0) {
    auto err = ps_cli_endpoint_unpack_err(f);
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(err);
  }
  auto decay = ps_decay_from_json(decay_json);
  flux_future_destroy(f);
  flux_close(h);
  return decay;
}

ps_result_t<ps_success_t> ps_cli_endpoint_flux_decay_reset() {
  auto h = flux_open(NULL, 0);
  if (!h) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }
  auto f =
      flux_rpc(h, "priorisea.daemon.decay.reset", NULL, FLUX_NODEID_ANY, 0);
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
ps_cli_endpoint_flux_decay_update(const ps_decay_t &decay) {
  auto h = flux_open(NULL, 0);
  if (!h) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }
  json_t *decay_json = ps_decay_to_json(decay);
  auto *f = flux_rpc_pack(h, "priorisea.daemon.decay.update", FLUX_NODEID_ANY,
                          0, "{s:O}", "data", decay_json);
  json_decref(decay_json);
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
