// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "error.hpp"
#include "types.hpp"
#include <flux/core.h>
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

////

ps_result_t<ps_db_jobs_get_t>
ps_db_jobs_get(const std::vector<flux_jobid_t> &jobids);

ps_result_t<ps_success_t> ps_db_t_inactive_set(double t_inactive);

ps_result_t<double> ps_db_t_inactive_get();

ps_result_t<ps_success_t> ps_db_job_set(const ps_job_t &job);

ps_result_t<ps_success_t> ps_db_score_init();

ps_result_t<ps_db_users_get_t>
ps_db_users_get(const std::vector<uid_t> &userids);

ps_result_t<ps_success_t> ps_db_user_set(const ps_user_t &user);

ps_result_t<ps_success_t>
ps_db_job_update(ps_flux_job_info_t job_info, const ps_formula_t &formula,
                 const std::vector<ps_timewin_t> &timewins);

ps_result_t<ps_db_users_get_t> ps_db_users_list();

ps_result_t<ps_db_jobs_get_t> ps_db_jobs_list();

ps_result_t<ps_success_t> ps_db_users_update();
