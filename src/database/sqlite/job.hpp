#pragma once
#include "src/database/job.hpp"
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include "src/lib/util.hpp"
#include <flux/core.h>
#include <sqlite3.h>
#include <sys/types.h>
#include <vector>

/**
 * Fetches a single job by id
 *
 * @param db The SQLite database
 * @param jobid The id of the job to fetch
 * @return The job, or an error if it does not exist
 */
ps_result_t<ps_job_t> ps_db_sqlite_job_get(sqlite3 *db, flux_jobid_t jobid);

/**
 * Fetches multiple jobs by id
 *
 * Returns one entry per requested id, with an error for any missing id
 *
 * @param db The SQLite database
 * @param jobids The ids of the jobs to fetch
 * @return A map of id to result
 */
ps_result_t<ps_db_sqlite_jobs_get_t>
ps_db_sqlite_jobs_get(sqlite3 *db, const std::vector<flux_jobid_t> &jobids);

/**
 * Lists the most recent jobs
 *
 * @param db The SQLite database
 * @param limit The maximum number of jobs to return
 * @param order The sort order of the listing (defaults to descending)
 * @return The jobs
 */
ps_result_t<std::vector<ps_job_t>>
ps_db_sqlite_jobs_list(sqlite3 *db, size_t limit,
                       ps_db_order_t order = ps_db_order_t::desc);

/**
 * Lists the most recent jobs owned by a user
 *
 * @param db The SQLite database
 * @param limit The maximum number of jobs to return
 * @param userid The owner id to filter by
 * @param order The sort order of the listing (defaults to descending)
 * @return The jobs
 */
ps_result_t<std::vector<ps_job_t>>
ps_db_sqlite_jobs_list(sqlite3 *db, size_t limit, uid_t userid,
                       ps_db_order_t order = ps_db_order_t::desc);

/**
 * Lists the jobs inactive for at least min_t_inactive
 *
 * Also returns, per user, the maximum job duration in the listing
 *
 * @param db The SQLite database
 * @param min_t_inactive The minimum t_inactive of the jobs to list
 * @param order The sort order of the listing (defaults to descending)
 * @return The jobs and the per-user maximum duration
 */
ps_result_t<ps_db_sqlite_jobs_list_t>
ps_db_sqlite_jobs_list(sqlite3 *db, double min_t_inactive,
                       ps_db_order_t order = ps_db_order_t::desc);

/**
 * Lists the jobs inactive for at least min_t_inactive owned by one of the
 * given user ids
 *
 * Also returns, per user, the maximum job duration in the listing
 *
 * @param db The SQLite database
 * @param min_t_inactive The minimum t_inactive of the jobs to list
 * @param userids The owner ids to filter by
 * @param order The sort order of the listing (defaults to descending)
 * @return The jobs and the per-user maximum duration
 */
ps_result_t<ps_db_sqlite_jobs_list_t>
ps_db_sqlite_jobs_list(sqlite3 *db, double min_t_inactive,
                       const std::vector<uid_t> &userids,
                       ps_db_order_t order = ps_db_order_t::desc);

/**
 * Inserts or replaces the jobs in a single transaction
 *
 * @param db The SQLite database
 * @param jobs The jobs to store
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t>
ps_db_sqlite_jobs_set(sqlite3 *db, const std::vector<ps_job_t> &jobs);

/**
 * Stores the jobs and advances the t_inactive cursor in a single transaction
 *
 * Both writes commit or roll back together so the jobs table and the cursor
 * can never diverge
 *
 * @param db The SQLite database
 * @param jobs The jobs to store
 * @param t_inactive The new t_inactive cursor value
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t>
ps_db_sqlite_jobs_set(sqlite3 *db, const std::vector<ps_job_t> &jobs,
                      double t_inactive);

/**
 * Inserts or replaces a single job in a single transaction
 *
 * @param db The SQLite database
 * @param job The job to store
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t> ps_db_sqlite_job_set(sqlite3 *db,
                                               const ps_job_t &job);

/**
 * Inserts new jobs, failing if any of them already exists
 *
 * @param db The SQLite database
 * @param jobs The jobs to insert
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t>
ps_db_sqlite_jobs_insert(sqlite3 *db, const std::vector<ps_job_t> &jobs);

/**
 * Inserts a new job, failing if it already exists
 *
 * @param db The SQLite database
 * @param job The job to insert
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t> ps_db_sqlite_job_insert(sqlite3 *db,
                                                  const ps_job_t &job);

/**
 * Updates existing jobs, failing if any of them is missing
 *
 * @param db The SQLite database
 * @param jobs The jobs to update
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t>
ps_db_sqlite_jobs_update(sqlite3 *db, const std::vector<ps_job_t> &jobs);

/**
 * Updates an existing job, failing if it is missing
 *
 * @param db The SQLite database
 * @param job The job to update
 * @return A result indicating the success of the operation
 */
ps_result_t<ps_success_t> ps_db_sqlite_job_update(sqlite3 *db,
                                                  const ps_job_t &job);
