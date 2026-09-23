// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <flux/core/job.h>
#include <map>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

//// timewin

struct ps_timewin_t {
  std::string name;
  std::string cron;
  int duration;
  double score;
};

//// formula

struct ps_formula_t {
  std::string name;
  std::string expr;
  std::map<std::string, double> defaults;
};

//// score

struct ps_job_t {
  flux_jobid_t jobid;
  double t_inactive;
  double score;
};

struct ps_user_t {
  uid_t userid;
  double score;
};

typedef std::map<uid_t, double> ps_score_map_t;

//// decay

struct ps_decay_frame_t {
  int duration;
  double priority;
};

typedef std::vector<ps_decay_frame_t> ps_decay_t;

//// flux

struct ps_flux_job_info_t {
  flux_jobid_t id;
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

// config

struct ps_seanerbus_t {
  std::string ip;
  int port;
};

struct ps_config_t {
  ps_seanerbus_t seanerbus;
};
