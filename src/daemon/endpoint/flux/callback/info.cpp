#include "src/daemon/endpoint/flux/callback/info.hpp"
#include "src/database/sqlite/info.hpp"
#include "src/database/sqlite/sqlite.hpp"
#include "src/lib/error.hpp"
#include "src/lib/info.hpp"

int ps_daemon_endpoint_flux_info_get(
    const ps_daemon_endpoint_flux_callback_args_t &args) {

  auto version_res = ps_db_sqlite_user_version_get(args.db);
  if (!ps_is_ok(version_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(version_res).c_str());
  }
  auto version = ps_unwrap(version_res);
  auto info_res = ps_db_sqlite_info_get(args.db);
  if (!ps_is_ok(info_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(info_res).c_str());
  }
  auto info = ps_unwrap(info_res);
  auto *info_json = ps_db_info_to_json(info);
  return flux_respond_pack(args.h, args.msg, "{s:{s:i s:s s:o}}", "data",
                           "version", version, "seanerbus_url",
                           args.seanerbus_url.c_str(), "info", info_json);
}

int ps_daemon_endpoint_flux_info_user_period_update(
    const ps_daemon_endpoint_flux_callback_args_t &args) {
  double user_period;
  if (flux_request_unpack(args.msg, NULL, "{s:f}", "data", &user_period) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }
  auto update_res = ps_db_sqlite_info_user_period_update(args.db, user_period);
  if (!ps_is_ok(update_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(update_res).c_str());
  }
  return flux_respond(args.h, args.msg, NULL);
}
