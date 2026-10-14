// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/daemon/endpoint/flux/callback/job.hpp"
#include "src/database/sqlite/job.hpp"
#include "src/lib/error.hpp"
#include "src/lib/job.hpp"
#include <flux/core.h>
#include <jansson.h>
#include <string>

void ps_daemon_endpoint_flux_job_get(flux_t *h, const flux_msg_t *msg,
                                     sqlite3 *db) {
  // Retrieve the job id from the payload
  const char *jobid_str;
  if (flux_request_unpack(msg, NULL, "{s:s}", "data", &jobid_str) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
    return;
  }

  // Parse the job id
  flux_jobid_t jobid;
  if (flux_job_id_parse(jobid_str, &jobid) < 0) {
    auto err = ps_flux_jobid_parse_err_t{.jobid = jobid_str,
                                         .strerror = strerror(errno)};
    flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
    return;
  }

  // Retrieve the job from the database
  auto get_res = ps_db_sqlite_job_get(db, jobid);
  if (!ps_is_ok(get_res)) {
    flux_respond_error(h, msg, 0, ps_error_message(get_res).c_str());
    return;
  }
  auto job = ps_unwrap(get_res);

  // Authorize: owners may read any job, others only their own
  uint32_t userid;
  uint32_t rolemask;
  if (flux_msg_get_userid(msg, &userid) < 0 ||
      flux_msg_get_rolemask(msg, &rolemask) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
    return;
  }
  if (!(rolemask & FLUX_ROLE_OWNER) &&
      job.userid != static_cast<uid_t>(userid)) {
    auto err = ps_access_denied_t{.key = "job." + std::string(jobid_str)};
    flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
    return;
  }

  flux_respond_pack(h, msg, "{s:o}", "data", ps_job_to_json(job));
}

void ps_daemon_endpoint_flux_job_list(flux_t *h, const flux_msg_t *msg,
                                      sqlite3 *db) {
  // Identify the caller: owners may list every job, others only their own
  uint32_t userid;
  uint32_t rolemask;
  if (flux_msg_get_userid(msg, &userid) < 0 ||
      flux_msg_get_rolemask(msg, &rolemask) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
    return;
  }
  auto is_owner = (rolemask & FLUX_ROLE_OWNER) != 0;

  // Retrieve the optional array of job ids from the payload
  json_t *data_json = nullptr;
  if (flux_request_unpack(msg, NULL, "{s?o}", "data", &data_json) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
    return;
  }

  std::vector<ps_job_t> jobs;
  if (data_json == nullptr) {
    // No parameter: return the authorized list
    ps_result_t<std::vector<ps_job_t>> list_res;
    if (is_owner) {
      list_res = ps_db_sqlite_jobs_list(db, SIZE_MAX);
    } else {
      // Non-owners only see their own jobs
      list_res = ps_db_sqlite_jobs_list(db, SIZE_MAX, userid);
    }
    if (!ps_is_ok(list_res)) {
      flux_respond_error(h, msg, 0, ps_error_message(list_res).c_str());
      return;
    }
    jobs = ps_unwrap(list_res);
  } else {
    if (!json_is_array(data_json)) {
      auto err = ps_field_type_mismatch_t{.field = "data", .expected = "array"};
      flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
      return;
    }

    // Build the list of job ids
    std::vector<flux_jobid_t> jobids;
    size_t index;
    json_t *value;
    json_array_foreach(data_json, index, value) {
      if (!json_is_string(value)) {
        auto err = ps_field_type_mismatch_t{
            .field = "data[" + std::to_string(index) + "]",
            .expected = "string"};
        flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
        return;
      }
      std::string jobid_str = json_string_value(value);
      flux_jobid_t jobid;
      if (flux_job_id_parse(jobid_str.c_str(), &jobid) < 0) {
        auto err = ps_flux_jobid_parse_err_t{.jobid = jobid_str,
                                             .strerror = strerror(errno)};
        flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
        return;
      }
      jobids.push_back(jobid);
    }

    // Retrieve the jobs from the database
    auto get_res = ps_db_sqlite_jobs_get(db, jobids);
    if (!ps_is_ok(get_res)) {
      flux_respond_error(h, msg, 0, ps_error_message(get_res).c_str());
      return;
    }
    auto jobs_map = ps_unwrap(get_res);

    // Build the list of jobs, preserving the request order
    for (const auto &jobid : jobids) {
      // The job is guaranteed to be in the map
      auto &job_res = jobs_map[jobid];
      if (!ps_is_ok(job_res)) {
        flux_respond_error(h, msg, 0, ps_error_message(job_res).c_str());
        return;
      }
      auto job = ps_unwrap(job_res);
      // Non-owners can only see their own jobs
      if (!is_owner && job.userid != static_cast<uid_t>(userid)) {
        auto err =
            ps_access_denied_t{.key = "job." + std::to_string(job.jobid)};
        flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
        return;
      }
      jobs.push_back(job);
    }
  }

  // Return as an array of jobs
  json_t *jobs_json = json_array();
  for (const auto &job : jobs) {
    json_array_append(jobs_json, ps_job_to_json(job));
  }
  flux_respond_pack(h, msg, "{s:o}", "data", jobs_json);
}

void ps_daemon_endpoint_flux_job_update(flux_t *h, const flux_msg_t *msg,
                                        sqlite3 *db) {}
