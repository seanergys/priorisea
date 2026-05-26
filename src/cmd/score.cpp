// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "score.hpp"
#include "lib/table.hpp"
#include "tinyexpr.h"
#include <flux/core.h>
#include <iostream>
#include <jansson.h>
#include <string>
#include <vector>

struct job {
  std::string jobid;
  int score;
};

struct user {
  std::string userid;
  int score;
};

static const std::string USAGE_MSG = "Usage: priorisea score [-h] COMMAND ARGS";
static const std::string TRY_HELP_MSG =
    "Try 'priorisea score -h' for more information.";
static const std::string HELP_MSG = USAGE_MSG + R"(

Manage and retrieve priority scores.
Example: priorisea score job 12345

COMMAND list and interpretation:
  update: update score values
  job: get score values for jobs
  user: get score values for users)";

static const std::string UPDATE_USAGE_MSG =
    "Usage: priorisea score update [-h] [OPTIONS] [JOBID...]";
static const std::string UPDATE_TRY_HELP_MSG =
    "Try 'priorisea score update -h' for more information.";
static const std::string UPDATE_HELP_MSG = UPDATE_USAGE_MSG + R"(

Update priority scores for jobs.
Example: priorisea score update 12345 67890
Example: priorisea score update --new

OPTIONS list and interpretation:
  --new: update scores only for new jobs)";

static int run_update(int argc, char *argv[]) {
  bool new_flag = false;
  std::vector<std::string> job_ids;

  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << UPDATE_HELP_MSG << std::endl;
    return 0;
  }

  if (argc < 1) {
    std::cout << UPDATE_USAGE_MSG << '\n' << UPDATE_TRY_HELP_MSG << std::endl;
    return -1;
  }

  for (int i = 0; i < argc; i++) {
    std::string arg = argv[i];

    if (arg == "--new") {
      new_flag = true;
    } else if (!arg.empty() && arg[0] == '-') {
      std::cout << "Error: 'priorisea score update " << arg
                << "' is not a valid option" << '\n'
                << UPDATE_USAGE_MSG << '\n'
                << UPDATE_TRY_HELP_MSG << std::endl;
      return -1;
    } else {
      job_ids.push_back(arg);
    }
  }

  if (job_ids.empty() && !new_flag) {
    std::cout << "Error: 'priorisea score update ...'" << '\n'
              << "  job id or --new is missing" << '\n'
              << UPDATE_USAGE_MSG << '\n'
              << UPDATE_TRY_HELP_MSG << std::endl;
    return -1;
  }

  return 0;
}

static const std::string JOB_USAGE_MSG =
    "Usage: priorisea score job [-h] [OPTIONS] JOBID [JOBID...]";
static const std::string JOB_TRY_HELP_MSG =
    "Try 'priorisea score job -h' for more information.";
static const std::string JOB_HELP_MSG = JOB_TRY_HELP_MSG + R"(

Retrieve score values for one or more jobs.
Example: priorisea score job 12345 67890

OPTIONS list and interpretation:
  --json: output in JSON format)";

static int run_job(int argc, char *argv[]) {
  bool json_flag = false;
  std::vector<std::string> job_ids;

  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << JOB_HELP_MSG << std::endl;
    return 0;
  }

  if (argc < 1) {
    std::cout << JOB_USAGE_MSG << '\n' << JOB_TRY_HELP_MSG << std::endl;
    return -1;
  }

  for (int i = 0; i < argc; i++) {
    std::string arg = argv[i];

    if (arg == "--json") {
      json_flag = true;
    } else if (!arg.empty() && arg[0] == '-') {
      std::cout << "Error: 'priorisea score job " << arg
                << "' is not a valid option" << '\n'
                << JOB_USAGE_MSG << '\n'
                << JOB_TRY_HELP_MSG << std::endl;
      return -1;
    } else {
      job_ids.push_back(arg);
    }
  }

  if (job_ids.empty()) {
    std::cout << "Error: 'priorisea score job ...'" << '\n'
              << "  job id is missing" << '\n'
              << JOB_USAGE_MSG << '\n'
              << JOB_TRY_HELP_MSG << std::endl;
    return -1;
  }

  std::vector<job> jobs = {
      {.jobid = "00000", .score = 0},
      {.jobid = "12345", .score = 1},
  };

  if (json_flag) {
    auto obj = json_object();
    for (const auto &f : jobs) {
      json_object_set_new(obj, f.jobid.c_str(), json_integer(f.score));
    }
    std::cout << json_dumps(obj, JSON_INDENT(2)) << std::endl;
    json_decref(obj);
  } else {
    print_table(jobs,
                {{"Job Id",
                  [](auto &value, auto index) -> auto { return value.jobid; }},
                 {"Score", [](auto &value, auto index) -> auto {
                    return std::to_string(value.score);
                  }}});
  }
  return 0;
}

static const std::string USER_USAGE_MSG =
    "Usage: priorisea score user [-h] [OPTIONS] USERID [USERID...]";
static const std::string USER_TRY_HELP_MSG =
    "Try 'priorisea score user -h' for more information.";
static const std::string USER_HELP_MSG = USER_USAGE_MSG + R"(

Retrieve score values for one or more users.
Example: priorisea score user alice bob

OPTIONS list and interpretation:
  --json: output in JSON format)";

static int run_user(int argc, char *argv[]) {
  bool json_flag = false;
  std::vector<std::string> user_ids;

  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << USER_HELP_MSG << std::endl;
    return 0;
  }

  if (argc < 1) {
    std::cout << USER_USAGE_MSG << '\n' << USER_TRY_HELP_MSG << std::endl;
    return -1;
  }

  for (int i = 0; i < argc; i++) {
    std::string arg = argv[i];

    if (arg == "--json") {
      json_flag = true;
    } else if (!arg.empty() && arg[0] == '-') {
      std::cout << "Error: 'priorisea score user " << arg
                << "' is not a valid option" << '\n'
                << USER_USAGE_MSG << '\n'
                << USER_TRY_HELP_MSG << std::endl;
      return -1;
    } else {
      user_ids.push_back(arg);
    }
  }

  if (user_ids.empty()) {
    std::cout << "Error: 'priorisea score user ...'" << '\n'
              << "  user id is missing" << '\n'
              << USER_USAGE_MSG << '\n'
              << USER_TRY_HELP_MSG << std::endl;
    return -1;
  }

  std::vector<user> users = {
      {.userid = "00000", .score = 0},
      {.userid = "12345", .score = 1},
  };

  if (json_flag) {
    auto obj = json_object();
    for (const auto &f : users) {
      json_object_set_new(obj, f.userid.c_str(), json_integer(f.score));
    }
    std::cout << json_dumps(obj, JSON_INDENT(2)) << std::endl;
    json_decref(obj);
  } else {
    print_table(users,
                {{"User Id",
                  [](auto &value, auto index) -> auto { return value.userid; }},
                 {"Score", [](auto &value, auto index) -> auto {
                    return std::to_string(value.score);
                  }}});
  }
  return 0;
}

int run_score(int argc, char *argv[]) {
  if (argc < 1) {
    std::cout << USAGE_MSG << '\n' << TRY_HELP_MSG << std::endl;
    return -1;
  }

  std::string cmd = argv[0];

  if (cmd == "-h") {
    std::cout << HELP_MSG << std::endl;
    return 0;
  } else if (cmd == "update") {
    return run_update(argc - 1, argv + 1);
  } else if (cmd == "job") {
    return run_job(argc - 1, argv + 1);
  } else if (cmd == "user") {
    return run_user(argc - 1, argv + 1);
  } else {
    std::cout << "Error: 'priorisea score " << cmd << "' is not a valid command"
              << '\n'
              << USAGE_MSG << '\n'
              << TRY_HELP_MSG << std::endl;
    return -1;
  }
}

bool validate_score(std::string formula, std::map<std::string, double> &vars) {
  auto *te_vars = (te_variable *)malloc(vars.size() * sizeof(te_variable));
  int i = 0;
  for (const auto &[name, value] : vars) {
    te_vars[i++] = {name.c_str(), &value};
  }
  int err = 0;
  te_expr *compiled = te_compile(formula.c_str(), te_vars, i, &err);
  if (!compiled)
    return false;
  te_free(compiled);
  return true;
}
