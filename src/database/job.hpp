#pragma once
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <flux/core.h>
#include <map>

/**
 * Map of job id to its get result, one entry per requested id
 */
using ps_db_sqlite_jobs_get_t = std::map<flux_jobid_t, ps_result_t<ps_job_t>>;

/**
 * Listing of jobs plus, for each user, the maximum job duration among
 * the listed jobs
 */
struct ps_db_sqlite_jobs_list_t {
  std::vector<ps_job_t> jobs;
  std::map<uid_t, double> max_duration_by_user;
};
