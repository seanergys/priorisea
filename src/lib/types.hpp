// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <flux/core.h>
#include <map>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

/**
 * Represents a time recurring window where jobs can be scheduled.
 */
struct ps_timewin_t {
  std::string name;
  std::string cron;
  int duration;
  double score;
};

/**
 * Represents a formula that can be used to calculate a double.
 */
struct ps_formula_t {
  std::string name;
  std::string expr;
  std::map<std::string, double> defaults;
};

/**
 * Represents a job with its associated information.
 */
struct ps_job_t {
  flux_jobid_t jobid;
  uid_t userid;
  double t_inactive;
  double score;
};

/**
 * Represents a user with their associated information.
 */
struct ps_user_t {
  std::string username;
  uid_t userid;
  double score;
};

/**
 * Represents a decay frame with a duration and weight.
 */
struct ps_decay_frame_t {
  int duration;
  double weight;
};

/**
 * Represents a decay configuration consisting of multiple decay frames.
 */
typedef std::vector<ps_decay_frame_t> ps_decay_t;

/**
 * Represents detailed information about a Flux job.
 */
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

struct ps_daemon_config_t {
  std::string database_path;
  std::string seanerbus_path;
};
