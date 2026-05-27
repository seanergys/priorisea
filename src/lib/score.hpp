// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "formula.hpp"
#include <flux/core.h>
#include <map>
#include <string>
#include <vector>

////

struct job {
  std::string jobid;
  int score;
};

struct user {
  int userid;
  int score;
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
