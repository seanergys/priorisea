#pragma once
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <flux/core.h>
#include <string>
#include <vector>

/**
 * Fetch a single job by id from the daemon
 *
 * @param jobid Id of the job to fetch
 * @param format Job listing format requested from the daemon
 */
ps_result_t<ps_job_t> ps_cli_endpoint_flux_job_get(const std::string &jobid,
                                                   const std::string &format);

/**
 * Fetch jobs from the daemon
 *
 * @param jobids Ids to fetch, or empty for all
 * @param format Job listing format requested from the daemon
 * @param limit Maximum number of jobs to return
 */
ps_result_t<std::vector<ps_job_t>>
ps_cli_endpoint_flux_job_list(const std::vector<std::string> &jobids,
                              const std::string &format, int limit);

/**
 * Mark jobs as new or not new on the daemon
 *
 * @param jobids Ids to update, or empty for all
 * @param _new New-flag value to apply
 */
ps_result_t<ps_success_t>
ps_cli_endpoint_flux_job_update(const std::vector<std::string> &jobids,
                                bool _new);
