#include "src/cli/endpoint/flux/job.hpp"
#include "src/cli/endpoint/util.hpp"
#include "src/lib/error.hpp"
#include "src/lib/job.hpp"
#include "src/lib/util.hpp"
#include <flux/core.h>
#include <jansson.h>
#include <string>

ps_result_t<ps_job_t> ps_cli_endpoint_flux_job_get(const std::string &jobid,
                                                   const std::string &format) {
  return ps_cli_endpoint_flux_rpc<ps_job_t>(
      [&](flux_t *h) {
        // The requested job-id format is always forwarded to the daemon.
        return flux_rpc_pack(h, "priorisea.daemon.job.get", FLUX_NODEID_ANY, 0,
                             "{s:s s:s}", "data", jobid.c_str(), "format",
                             format.c_str());
      },
      [&](flux_future_t *f) -> ps_result_t<ps_job_t> {
        json_t *job_json;
        if (flux_rpc_get_unpack(f, "{s:o}", "data", &job_json) < 0) {
          return ps_error(ps_cli_endpoint_unpack_err(f));
        }
        return ps_job_from_json(job_json);
      });
}

ps_result_t<std::vector<ps_job_t>> ps_cli_endpoint_flux_job_list(
    const std::vector<std::string> &jobids, const std::string &format,
    int limit, const ps_db_order_t &order, const ps_job_sort_t &sort) {
  auto order_str = ps_db_order_to_str(order);
  auto sort_str = ps_job_sort_to_str(sort);
  return ps_cli_endpoint_flux_rpc<std::vector<ps_job_t>>(
      [&](flux_t *h) -> flux_future_t * {
        if (jobids.empty()) {
          // No parameter: request the entire list
          return flux_rpc_pack(h, "priorisea.daemon.job.list", FLUX_NODEID_ANY,
                               0, "{s:s s:i s:s s:s}", "format", format.c_str(),
                               "limit", limit, "order", order_str.c_str(),
                               "sort", sort_str.c_str());
        }
        json_t *data = json_array();
        for (const auto &jobid : jobids) {
          json_array_append_new(data, json_string(jobid.c_str()));
        }
        auto *f =
            flux_rpc_pack(h, "priorisea.daemon.job.list", FLUX_NODEID_ANY, 0,
                          "{s:O s:s s:i s:s s:s}", "data", data, "format",
                          format.c_str(), "limit", limit, "order",
                          order_str.c_str(), "sort", sort_str.c_str());
        json_decref(data);
        return f;
      },
      [&](flux_future_t *f) -> ps_result_t<std::vector<ps_job_t>> {
        json_t *jobs_json;
        if (flux_rpc_get_unpack(f, "{s:o}", "data", &jobs_json) < 0) {
          return ps_error(ps_cli_endpoint_unpack_err(f));
        }
        auto jobs_res = ps_jobs_from_json(jobs_json);
        if (!ps_is_ok(jobs_res)) {
          return ps_forward_error<std::vector<ps_job_t>>(jobs_res);
        }
        return ps_unwrap(jobs_res);
      });
}

ps_result_t<ps_success_t>
ps_cli_endpoint_flux_job_update(const std::vector<std::string> &jobids,
                                bool _new) {
  return ps_cli_endpoint_flux_rpc<ps_success_t>(
      [&](flux_t *h) -> flux_future_t * {
        if (jobids.empty()) {
          return flux_rpc_pack(h, "priorisea.daemon.job.update",
                               FLUX_NODEID_ANY, 0, "{s:{s:b}}", "data", "new",
                               _new);
        }
        json_t *data = json_array();
        for (const auto &jobid : jobids) {
          json_array_append_new(data, json_string(jobid.c_str()));
        }
        auto *f =
            flux_rpc_pack(h, "priorisea.daemon.job.update", FLUX_NODEID_ANY, 0,
                          "{s:{s:b s:O}}", "data", "new", _new, "jobids", data);
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
