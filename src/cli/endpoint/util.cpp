// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/cli/endpoint/util.hpp"

ps_error_kind_t ps_cli_endpoint_unpack_err(flux_future_t *f) {
  ps_error_kind_t err;
  auto err_str = flux_future_error_string(f);
  if (err_str) {
    err = ps_daemon_err_t{.err = std::string(err_str)};
  } else {
    err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
  }
  return err;
}
