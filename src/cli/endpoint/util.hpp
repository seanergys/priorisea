#pragma once
#include "src/lib/error.hpp"
#include <flux/core.h>
#include <functional>

/**
 * Extract the error kind from a flux future
 *
 * @param f Future to inspect
 *
 * @returns A corresponding error kind
 */
ps_error_kind_t ps_cli_endpoint_unpack_err(flux_future_t *f);

/**
 * Run a request/response flux RPC against the daemon
 *
 * @param pack   Builds the future for the request from the handle
 *               Returning nullptr is treated as a request-build failure and
 *               mapped to a ps_flux_rpc_err_t
 * @param unpack Consumes the future: unpack the response and parse it into T
 *
 * @returns A ps_result_t containing the unpacked response or an error.
 */
template <typename T>
ps_result_t<T> ps_cli_endpoint_flux_rpc(
    const std::function<flux_future_t *(flux_t *)> &pack,
    const std::function<ps_result_t<T>(flux_future_t *)> &unpack) {
  auto h = flux_open(NULL, 0);
  if (!h) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }
  auto *f = pack(h);
  if (!f) {
    flux_close(h);
    return ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
  }
  auto res = unpack(f);
  flux_future_destroy(f);
  flux_close(h);
  return res;
}
