#pragma once

#include <flux/core.h>
#include <map>
#include <optional>
#include <string>
#include <sys/types.h>
#include <unistd.h>

/**
 * Represents a time recurring window where jobs can be scheduled
 */
struct ps_timewin_t {
  std::string name;
  std::string cron;
  int duration;
  double score;
};

/**
 * Represents a formula that can be used to calculate a double
 */
struct ps_formula_t {
  std::string name;
  std::string expr;
  std::map<std::string, double> defaults;
};

/**
 * Represents a job with its associated information
 */
struct ps_job_t {
  std::string jobid;
  uid_t userid;
  double t_inactive;
  double score;
  double duration;
  std::string s_expr;
  std::map<std::string, double> s_defaults;
  std::map<std::string, double> s_values;
  std::optional<double> weight;
  std::optional<std::map<std::string, double>> w_values;
};

/**
 * Represents a user with their associated information
 */
struct ps_user_t {
  uid_t userid;
  std::optional<double> score;
  std::string w_expr;
  std::map<std::string, double> w_defaults;
  double t_executed;
  double t_inactive_min;
};

/**
 * Represents the database's info row: the t_inactive cursor and the default
 * user_period
 */
struct ps_db_info_t {
  double t_inactive;
  double user_period;
};

/**
 * Configuration for running the daemon
 */
struct ps_daemon_config_t {
  std::string database_path;
  std::string seanerbus_url;
};
