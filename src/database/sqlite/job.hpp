// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/database/job.hpp"
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <flux/core.h>
#include <sqlite3.h>
#include <sys/types.h>
#include <vector>

ps_result_t<ps_job_t> ps_db_sqlite_job_get(sqlite3 *db, flux_jobid_t jobid);

ps_result_t<ps_db_sqlite_jobs_get_t>
ps_db_sqlite_jobs_get(sqlite3 *db, const std::vector<flux_jobid_t> &jobids);

ps_result_t<std::vector<ps_job_t>> ps_db_sqlite_jobs_list(sqlite3 *db,
                                                          size_t limit);

ps_result_t<std::vector<ps_job_t>>
ps_db_sqlite_jobs_list(sqlite3 *db, size_t limit, uid_t userid);

ps_result_t<std::vector<ps_job_t>>
ps_db_sqlite_jobs_list(sqlite3 *db, double min_t_inactive);

// Returns the jobs with t_inactive >= min_t_inactive whose owner is one of the
// given user ids, most recent first (higher t_inactive sorts earlier).
ps_result_t<std::vector<ps_job_t>>
ps_db_sqlite_jobs_list(sqlite3 *db, double min_t_inactive,
                       const std::vector<uid_t> &userids);

ps_result_t<ps_success_t>
ps_db_sqlite_jobs_set(sqlite3 *db, const std::vector<ps_job_t> &jobs);

// Atomically stores the job scores and advances the t_inactive cursor in a
// single transaction: either both commit or both roll back, so the jobs table
// and the cursor can never diverge.
ps_result_t<ps_success_t>
ps_db_sqlite_jobs_set(sqlite3 *db, const std::vector<ps_job_t> &jobs,
                      double t_inactive);

ps_result_t<ps_success_t> ps_db_sqlite_job_set(sqlite3 *db,
                                               const ps_job_t &job);

ps_result_t<ps_success_t>
ps_db_sqlite_jobs_insert(sqlite3 *db, const std::vector<ps_job_t> &jobs);

ps_result_t<ps_success_t> ps_db_sqlite_job_insert(sqlite3 *db,
                                                  const ps_job_t &job);

ps_result_t<ps_success_t>
ps_db_sqlite_jobs_update(sqlite3 *db, const std::vector<ps_job_t> &jobs);

ps_result_t<ps_success_t> ps_db_sqlite_job_update(sqlite3 *db,
                                                  const ps_job_t &job);
