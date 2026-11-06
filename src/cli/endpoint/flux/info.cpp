#include "src/cli/endpoint/flux/info.hpp"
#include "src/cli/endpoint/error.hpp"
#include "src/cli/endpoint/util.hpp"
#include "src/lib/error.hpp"
#include "src/lib/info.hpp"
#include <flux/core/rpc.h>

ps_result_t<ps_cli_endpoint_info_get_t> ps_cli_endpoint_info_get() {
  return ps_cli_endpoint_flux_rpc<ps_cli_endpoint_info_get_t>(
      [&](flux_t *h) {
        return flux_rpc(h, "priorisea.daemon.info.get", NULL, FLUX_NODEID_ANY,
                        0);
      },
      [&](flux_future_t *f) -> ps_result_t<ps_cli_endpoint_info_get_t> {
        int version;
        const char *_seanerbus_url;
        json_t *info_json = nullptr;
        if (flux_rpc_get_unpack(f, "{s:{s:i s:s s:o}}", "data", "version",
                                &version, "seanerbus_url", &_seanerbus_url,
                                "info", &info_json) < 0) {
          return ps_error(ps_cli_endpoint_unpack_err(f));
        }
        auto seanerbus_url = std::string(_seanerbus_url);
        auto info_res = ps_db_info_from_json(info_json);
        if (!ps_is_ok(info_res)) {
          return ps_forward_error<ps_cli_endpoint_info_get_t>(info_res);
        }
        return ps_cli_endpoint_info_get_t{.version = version,
                                          .seanerbus_url = seanerbus_url,
                                          .info = ps_unwrap(info_res)};
      });
}

ps_result_t<ps_success_t>
ps_cli_endpoint_flux_user_period_update(double user_period) {
  return ps_cli_endpoint_flux_rpc<ps_success_t>(
      [&](flux_t *h) {
        return flux_rpc_pack(h, "priorisea.daemon.info.user_period.update",
                             FLUX_NODEID_ANY, 0, "{s:f}", "data", user_period);
      },
      [&](flux_future_t *f) -> ps_result_t<ps_success_t> {
        if (flux_rpc_get(f, NULL) < 0) {
          return ps_error(
              ps_daemon_err_t{.err = std::string(flux_future_error_string(f))});
        }
        return ps_success_t{};
      });
}
