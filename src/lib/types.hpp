// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <map>
#include <string>
#include <vector>

//// timewin

struct ps_timewin_t {
  std::string name;
  std::string cron;
  int duration;
  double priority;
};

//// formula

struct ps_formula_t {
  std::string name;
  std::string expr;
  std::map<std::string, double> vars;
};

//// score

struct ps_job_t {
  std::string jobid;
  double score;
};

struct ps_user_t {
  int userid;
  double score;
};

typedef std::map<int, double> ps_score_map_t;

//// decay

struct ps_decay_one_t {
  int duration;
  double priority;
};

typedef std::vector<ps_decay_one_t> ps_decay_t;

//// flux

struct ps_flux_job_info_t {
  long long id;
  int userid;
  int urgency;
  int priority;
  double t_submit;
  double t_depend;
  double t_run;
  double t_cleanup;
  double t_inactive;
  int state;
  std::string name;
  std::string cwd;
  int ntasks;
  int ncores;
  double duration;
  int nnodes;
  std::string ranks;
  std::string nodelist;
  bool success;
  bool exception_occured;
  // std::string exception_type;
  // int exception_severity;
  // std::string exception_note;
  int result;
  double expiration;
  // json_t annotations;
  // int waitstatus;
};
