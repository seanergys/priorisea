// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "error.hpp"
#include "types.hpp"
#include <flux/core.h>
#include <jansson.h>
#include <vector>

struct ps_db_job_get_entry_t {
  flux_jobid_t jobid;
  ps_result_t<ps_job_t> result;
};
using ps_db_jobs_get_t = std::vector<ps_db_job_get_entry_t>;

struct ps_db_user_get_entry_t {
  uid_t userid;
  ps_result_t<ps_user_t> result;
};
using ps_db_users_get_t = std::vector<ps_db_user_get_entry_t>;

/**
 * Converts a job structure to a JSON object.
 *
 * @param job The job structure to convert.
 * @return A JSON object representing the job.
 */
json_t *ps_job_to_json(const ps_job_t &job);

/**
 * Converts a JSON object to a job structure.
 *
 * @param obj The JSON object to convert.
 * @return A result containing the converted job.
 */
ps_result_t<ps_job_t> ps_job_from_json(json_t *obj);

/**
 * Retrieves a list of jobs from the database.
 *
 * @param jobids The list of job IDs to retrieve.
 * @return A result containing the retrieved jobs.
 */
ps_result_t<ps_db_jobs_get_t>
ps_db_jobs_get(const std::vector<flux_jobid_t> &jobids);

/**
 * Sets the t_inactive value in the database.
 *
 * @param t_inactive The t_inactive value to set.
 * @return A result indicating success or failure.
 */
ps_result_t<ps_success_t> ps_db_t_inactive_set(double t_inactive);

/**
 * Retrieves the t_inactive value from the database.
 *
 * @return A result containing the t_inactive value.
 */
ps_result_t<double> ps_db_t_inactive_get();

/**
 * Sets a job in the database.
 *
 * @param job The job to set.
 * @return A result indicating success or failure.
 */
ps_result_t<ps_success_t> ps_db_job_set(const ps_job_t &job);

/**
 * Initializes the score database.
 *
 * @return A result indicating success or failure.
 */
ps_result_t<ps_success_t> ps_db_score_init();

/**
 * Retrieves a list of users from the database.
 *
 * @param userids The list of user IDs to retrieve.
 * @return A result containing the list of users.
 */
ps_result_t<ps_db_users_get_t>
ps_db_users_get(const std::vector<uid_t> &userids);

/**
 * Sets a user in the database.
 *
 * @param user The user to set.
 * @return A result indicating success or failure.
 */
ps_result_t<ps_success_t> ps_db_user_set(const ps_user_t &user);

/**
 * Updates a job in the database.
 *
 * @param job_info The job information to update.
 * @param formula The formula to compute the score.
 * @param timewins The list of time windows to apply.
 * @param seanerbus The seanerbus configuration to use.
 * @return A result indicating success or failure.
 */
ps_result_t<ps_success_t>
ps_db_job_update(ps_flux_job_info_t job_info, const ps_formula_t &formula,
                 const std::vector<ps_timewin_t> &timewins,
                 const ps_seanerbus_t &seanerbus);

/**
 * Lists all users in the database.
 *
 * @return A result containing the list of users.
 */
ps_result_t<ps_db_users_get_t> ps_db_users_list();

/**
 * Lists all jobs in the database.
 *
 * @return A result containing the list of jobs.
 */
ps_result_t<ps_db_jobs_get_t> ps_db_jobs_list();

/**
 * Lists all jobs in the database, ranked by their inactive time (most recent
 * first).
 *
 * @param limit The maximum number of jobs to retrieve.
 * @return A result containing the list of jobs.
 */
ps_result_t<ps_db_jobs_get_t> ps_db_jobs_list_ranked(size_t limit);

/**
 * Updates the database with the latest users information.
 *
 * @return A result indicating success or failure.
 */
ps_result_t<ps_success_t> ps_db_users_update();
