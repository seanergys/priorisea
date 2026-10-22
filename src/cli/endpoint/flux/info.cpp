// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/cli/endpoint/flux/info.hpp"
#include "src/cli/endpoint/util.hpp"
#include "src/lib/error.hpp"
#include "src/lib/info.hpp"

ps_result_t<ps_cli_endpoint_info_get_t> ps_cli_endpoint_info_get() {
  auto h = flux_open(NULL, 0);
  if (!h) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }
  auto f = flux_rpc(h, "priorisea.daemon.info.get", NULL, FLUX_NODEID_ANY, 0);
  if (!f) {
    flux_close(h);
    return ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
  }

  int version;
  json_t *info_json = nullptr;
  if (flux_rpc_get_unpack(f, "{s:{s:i s:o}}", "data", "version", &version,
                          "info", &info_json) < 0) {
    auto err = ps_cli_endpoint_unpack_err(f);
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(err);
  }
  auto info_res = ps_db_info_from_json(info_json);
  flux_future_destroy(f);
  flux_close(h);
  if (!ps_is_ok(info_res)) {
    return ps_forward_error<ps_cli_endpoint_info_get_t>(info_res);
  }
  return ps_cli_endpoint_info_get_t{.version = version,
                                    .info = ps_unwrap(info_res)};
}
