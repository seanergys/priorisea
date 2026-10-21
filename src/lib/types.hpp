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
  std::string jobid;
  uid_t userid;
  double t_inactive;
  double score;
};

/**
 * Represents a user with their associated information.
 */
struct ps_user_t {
  uid_t userid;
  double score;
};

/**
 * Represents the database's info row: the t_inactive cursor and the default
 * user_period. The two fields are always read and written together as a single
 * unit, so the row can never be left half-written (for example with a NULL
 * user_period, which the schema forbids).
 */
struct ps_db_info_t {
  double t_inactive;
  double user_period;
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

struct ps_daemon_config_t {
  std::string database_path;
  std::string seanerbus_url;
};
