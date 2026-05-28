// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "score.hpp"
#include "lib/flux.hpp"
#include "lib/score.hpp"
#include "lib/util.hpp"
#include <flux/core.h>
#include <iostream>
#include <jansson.h>
#include <string>
#include <vector>

////

static const std::string INIT_USAGE_MSG = "Usage: priorisea score init [-h]";
static const std::string INIT_TRY_HELP_MSG =
    "Try 'priorisea score init -h' for more information.";
static const std::string INIT_HELP_MSG = INIT_USAGE_MSG + R"(

Initialize score database for priorisea.)";
static int run_init(int argc, char *argv[]) {
  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << INIT_HELP_MSG << std::endl;
    return 0;
  }
  if (argc > 0) {
    std::cerr << INIT_USAGE_MSG << '\n' << INIT_TRY_HELP_MSG << std::endl;
    return -1;
  }

  init_score_res res = init_score();
  if (!res.ok) {
    std::cerr << "Error: 'priorisea score update ...'" << '\n'
              << "  " << res.message << '\n'
              << INIT_USAGE_MSG << '\n'
              << INIT_TRY_HELP_MSG << std::endl;
    return -1;
  }

  return 0;
}

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
  std::vector<std::string> jobids;

  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << UPDATE_HELP_MSG << std::endl;
    return 0;
  }

  if (argc < 1) {
    std::cerr << UPDATE_USAGE_MSG << '\n' << UPDATE_TRY_HELP_MSG << std::endl;
    return -1;
  }

  for (int i = 0; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--new") {
      new_flag = true;
    } else {
      jobids.push_back(arg);
    }
  }

  if (jobids.empty() && !new_flag) {
    std::cerr << "Error: 'priorisea score update ...'" << '\n'
              << "  job id or --new is missing" << '\n'
              << UPDATE_USAGE_MSG << '\n'
              << UPDATE_TRY_HELP_MSG << std::endl;
    return -1;
  }

  std::string formula_name = "score";
  auto get_formula_res = get_formula(formula_name);
  if (!get_formula_res.ok) {
    std::cerr << "Error: 'priorisea score update ...'" << '\n'
              << "  could not get formula: " << get_formula_res.message << '\n'
              << UPDATE_USAGE_MSG << '\n'
              << UPDATE_TRY_HELP_MSG << std::endl;
    return -1;
  }

  std::vector<flux_list_job_res> all_jobs;

  if (new_flag) {
    auto get_t_inactive_res = get_t_inactive();
    if (!get_t_inactive_res.ok) {
      std::cerr << "Error: 'priorisea score update ...'" << '\n'
                << "  " << get_t_inactive_res.message << '\n'
                << UPDATE_USAGE_MSG << '\n'
                << UPDATE_TRY_HELP_MSG << std::endl;
      return -1;
    }

    auto list_inactive_res =
        flux_list_inactive(get_t_inactive_res.t_inactive, 10);
    if (!list_inactive_res.ok) {
      std::cerr << "Error: 'priorisea score update ...'" << '\n'
                << "  " << list_inactive_res.message << '\n'
                << UPDATE_USAGE_MSG << '\n'
                << UPDATE_TRY_HELP_MSG << std::endl;
      return -1;
    }

    for (auto &get_inactive_job_res : list_inactive_res.jobs) {
      if (get_inactive_job_res.ok) {
        all_jobs.push_back(get_inactive_job_res.job);
      }
    }

    if (!all_jobs.empty()) {
      auto store_t_inactive_res =
          store_t_inactive(list_inactive_res.t_inactive_max);
      if (!store_t_inactive_res.ok) {
        std::cerr << "Error: 'priorisea score update ...'" << '\n'
                  << "  " << store_t_inactive_res.message << '\n'
                  << UPDATE_USAGE_MSG << '\n'
                  << UPDATE_TRY_HELP_MSG << std::endl;
        return -1;
      }
    }
  }

  if (jobids.size() > 0) {
    auto list_res = flux_list(jobids, 10);
    if (!list_res.ok) {
      std::cerr << "Error: 'priorisea score update ...'" << '\n'
                << "  " << list_res.message << '\n'
                << UPDATE_USAGE_MSG << '\n'
                << UPDATE_TRY_HELP_MSG << std::endl;
      return -1;
    }

    for (auto &get_job_res : list_res.jobs) {
      if (get_job_res.ok) {
        all_jobs.push_back(get_job_res.job);
      }
    }
  }

  // TODO: --new and specifying jobid can both find the
  // same job twice, thus updating it twice
  for (const auto &list_job : all_jobs) {
    std::map<std::string, double> vars_over;
    auto eval_res = eval_formula(get_formula_res.formula_obj, vars_over);
    if (!eval_res.ok) {
      std::cerr << "Error: failed to evaluate formula" << std::endl;
      continue;
    }

    auto store_job_res =
        store_job({std::to_string(list_job.id), eval_res.result});
    if (!store_job_res.ok) {
      std::cerr << "Error: " << store_job_res.message << std::endl;
      continue;
    }
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
  std::vector<std::string> jobids;

  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << JOB_HELP_MSG << std::endl;
    return 0;
  }

  if (argc < 1) {
    std::cerr << JOB_USAGE_MSG << '\n' << JOB_TRY_HELP_MSG << std::endl;
    return -1;
  }

  for (int i = 0; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--json") {
      json_flag = true;
    } else {
      jobids.push_back(arg);
    }
  }

  if (jobids.empty()) {
    std::cerr << "Error: 'priorisea score job ...'" << '\n'
              << "  job id is missing" << '\n'
              << JOB_USAGE_MSG << '\n'
              << JOB_TRY_HELP_MSG << std::endl;
    return -1;
  }

  auto get_res = get_jobs(jobids);
  if (!get_res.ok) {
    std::cerr << "Error: 'priorisea score job ...'" << '\n'
              << "  " << get_res.message << '\n'
              << JOB_USAGE_MSG << '\n'
              << JOB_TRY_HELP_MSG << std::endl;
    return -1;
  }

  std::vector<job> jobs;
  for (const auto &get_job_res : get_res.res) {
    if (get_job_res.ok) {
      jobs.push_back(get_job_res.job_obj);
    } else {
      std::cerr << "Error: " << get_job_res.message << std::endl;
    }
  }

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
      {.userid = 1000, .score = 0},
      {.userid = 1001, .score = 1},
  };

  if (json_flag) {
    auto obj = json_object();
    for (const auto &f : users) {
      json_object_set_new(obj, std::to_string(f.userid).c_str(),
                          json_integer(f.score));
    }
    std::cout << json_dumps(obj, JSON_INDENT(2)) << std::endl;
    json_decref(obj);
  } else {
    print_table(users, {{"UserId",
                         [](auto &value, auto index) -> auto {
                           return std::to_string(value.userid);
                         }},
                        {"Score", [](auto &value, auto index) -> auto {
                           return std::to_string(value.score);
                         }}});
  }
  return 0;
}

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
int run_score(int argc, char *argv[]) {
  if (argc < 1) {
    std::cout << USAGE_MSG << '\n' << TRY_HELP_MSG << std::endl;
    return -1;
  }

  std::string cmd = argv[0];

  if (cmd == "-h") {
    std::cout << HELP_MSG << std::endl;
    return 0;
  } else if (cmd == "init") {
    return run_init(argc - 1, argv + 1);
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
