// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/cli/endpoint/flux/info.hpp"
#include "src/cli/endpoint/util.hpp"

ps_result_t<ps_cli_endpoint_info_get_t>
ps_cli_endpoint_info_get() {
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
    double t_inactive;
    if (flux_rpc_get_unpack(f, "{s:{s:i s:f}}", "data", "version", &version, "t_inactive", &t_inactive) < 0) {
      auto err = ps_cli_endpoint_unpack_err(f);
      flux_future_destroy(f);
      flux_close(h);
      return ps_error(err);
    }
    flux_future_destroy(f);
    flux_close(h);
    return ps_cli_endpoint_info_get_t{.version = version, .t_inactive = t_inactive};
}
