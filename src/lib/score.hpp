// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "flux.hpp"
#include "formula.hpp"
#include <flux/core.h>
#include <map>
#include <string>
#include <vector>

////

struct job {
  std::string jobid;
  double score;
};

struct user {
  int userid;
  double score;
};

////

struct get_job_res {
  bool ok;
  std::string message;
  std::string jobid;
  job job_obj;
};
struct get_jobs_res {
  bool ok;
  std::string message;
  std::vector<get_job_res> res;
};
get_jobs_res get_jobs(std::vector<std::string> &jobids);

struct eval_formula_res {
  bool ok;
  double result;
};
eval_formula_res eval_formula(formula formula_obj,
                              std::map<std::string, double> &vars_over);

bool validate_score(formula formula_obj);

struct store_t_inactive_res {
  bool ok;
  std::string message;
};
store_t_inactive_res store_t_inactive(double t_inactive);

struct get_t_inactive_res {
  bool ok;
  std::string message;
  double t_inactive;
};
get_t_inactive_res get_t_inactive();

struct store_job_res {
  bool ok;
  std::string message;
};
store_job_res store_job(job job_obj);

struct init_score_res {
  bool ok;
  std::string message;
};
init_score_res init_score();

struct get_user_res {
  bool ok;
  std::string message;
  int userid;
  user user_obj;
};
struct get_users_res {
  bool ok;
  std::string message;
  std::vector<get_user_res> res;
};
get_users_res get_users(std::vector<int> &userids);

struct store_user_res {
  bool ok;
  std::string message;
};
store_user_res store_user(user user_obj);

struct update_job_res {
  bool ok;
  std::string message;
};
update_job_res update_job(flux_list_job_res job_obj, formula formula_obj);

struct update_users_res {
  bool ok;
  std::string message;
};
update_users_res update_users();
