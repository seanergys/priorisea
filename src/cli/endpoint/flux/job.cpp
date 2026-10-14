// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/cli/endpoint/flux/job.hpp"
#include "src/cli/endpoint/util.hpp"
#include "src/lib/error.hpp"
#include "src/lib/job.hpp"
#include <flux/core.h>
#include <jansson.h>
#include <string>

ps_result_t<ps_job_t> ps_cli_endpoint_flux_job_get(const std::string &jobid) {
  auto h = flux_open(NULL, 0);
  if (!h) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }
  auto *f = flux_rpc_pack(h, "priorisea.daemon.job.get", FLUX_NODEID_ANY, 0,
                          "{s:s}", "data", jobid.c_str());
  if (!f) {
    flux_close(h);
    return ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
  }

  json_t *job_json;
  if (flux_rpc_get_unpack(f, "{s:o}", "data", &job_json) < 0) {
    auto err = ps_cli_endpoint_unpack_err(f);
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(err);
  }
  auto job = ps_job_from_json(job_json);
  flux_future_destroy(f);
  flux_close(h);
  return job;
}

ps_result_t<std::vector<ps_job_t>>
ps_cli_endpoint_flux_job_list(const std::vector<std::string> &jobids) {
  auto h = flux_open(NULL, 0);
  if (!h) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }

  flux_future_t *f;
  if (jobids.empty()) {
    // No parameter: request the entire list
    // An empty object is sent
    f = flux_rpc_pack(h, "priorisea.daemon.job.list", FLUX_NODEID_ANY, 0, "{}");
  } else {
    json_t *data = json_array();
    for (const auto &jobid : jobids) {
      json_array_append_new(data, json_string(jobid.c_str()));
    }
    f = flux_rpc_pack(h, "priorisea.daemon.job.list", FLUX_NODEID_ANY, 0,
                      "{s:O}", "data", data);
    json_decref(data);
  }
  if (!f) {
    flux_close(h);
    return ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
  }

  json_t *jobs_json;
  if (flux_rpc_get_unpack(f, "{s:o}", "data", &jobs_json) < 0) {
    auto err = ps_cli_endpoint_unpack_err(f);
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(err);
  }

  std::vector<ps_job_t> jobs;
  size_t index;
  json_t *value;
  json_array_foreach(jobs_json, index, value) {
    auto job_res = ps_job_from_json(value);
    if (!ps_is_ok(job_res)) {
      flux_future_destroy(f);
      flux_close(h);
      return ps_forward_error<std::vector<ps_job_t>>(job_res);
    }
    jobs.push_back(ps_unwrap(job_res));
  }

  flux_future_destroy(f);
  flux_close(h);
  return jobs;
}
