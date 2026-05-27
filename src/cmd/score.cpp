// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "score.hpp"
#include "common/util.hpp"
#include "lib/kvs.hpp"
#include "lib/table.hpp"
#include "tinyexpr.h"
#include <flux/core.h>
#include <iostream>
#include <jansson.h>
#include <string>
#include <vector>
#include "formula.hpp"

struct job {
  std::string jobid;
  int score;
};
struct user {
  int userid;
  int score;
};

static double min(double a, double b) { return a < b ? a : b; }
static double max(double a, double b) { return a > b ? a : b; }

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
static get_jobs_res get_jobs(std::vector<std::string> &jobids) {
  get_jobs_res res;
  res.ok = false;

  std::vector<std::string> keys;
  for (const auto &jobid : jobids) {
    keys.push_back("priorisea.job." + jobid);
  }
  auto kvs_res = kvs_lookup(keys);
  if (kvs_res.err != KVS_ERR_SUCCESS) {
    res.message = "Failed to lookup KVS. Unknown error";
    return res;
  }

  for (const auto &lookup_key_res : kvs_res.res) {
    get_job_res job_res;
    job_res.ok = false;

    size_t pos = lookup_key_res.key.rfind('.');
    std::string jobid = (pos == std::string::npos)
                            ? lookup_key_res.key
                            : lookup_key_res.key.substr(pos + 1);
    job_res.jobid = jobid;

    if (lookup_key_res.err != KVS_ERR_SUCCESS) {
      job_res.message = "Failed to lookup KVS. Unknown error: " +
                        std::to_string(lookup_key_res.err);
      res.res.push_back(job_res);
      continue;
    }

    int score;
    if (!parse_int(lookup_key_res.value, score)) {
      job_res.message =
          "Failed to parse score from KVS value: " + lookup_key_res.value;
      res.res.push_back(job_res);
      continue;
    }

    job_res.ok = true;
    job_res.job_obj = {.jobid = jobid, .score = score};
    res.res.push_back(job_res);
  }

  res.ok = true;
  return res;
};

struct eval_formula_res {
  bool ok;
  double result;
};
static eval_formula_res eval_formula(formula formula_obj,
                                     std::map<std::string, double> &vars_over) {

  std::map<std::string, double> vars_merged = vars_over;
  for (const auto &[name, value] : formula_obj.vars) {
    if (vars_merged.find(name) != vars_merged.end())
      continue;
    vars_merged[name] = value;
  }

  auto *te_vars =
      (te_variable *)malloc((vars_merged.size() + 2) * sizeof(te_variable));
  int i = 0;

  for (const auto &[name, value] : vars_merged) {
    te_vars[i++] = {name.c_str(), &value};
  }
  te_vars[i++] = {"min", (void *)min, TE_FUNCTION2};
  te_vars[i++] = {"max", (void *)max, TE_FUNCTION2};

  int err = 0;
  te_expr *compiled = te_compile(formula_obj.expr.c_str(), te_vars, i, &err);
  if (!compiled)
    return {false, 0};
  auto result = te_eval(compiled);
  te_free(compiled);
  return {true, result};
}

////

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

  // TODO: remove once --new flag is implemented
  if (new_flag) {
    std::cerr << "Error: 'priorisea score update ...'" << '\n'
              << "  --new flag is not implemented" << '\n'
              << UPDATE_USAGE_MSG << '\n'
              << UPDATE_TRY_HELP_MSG << std::endl;
    return -1;
  }

  if (jobids.empty() && !new_flag) {
    std::cerr << "Error: 'priorisea score update ...'" << '\n'
              << "  job id or --new is missing" << '\n'
              << UPDATE_USAGE_MSG << '\n'
              << UPDATE_TRY_HELP_MSG << std::endl;
    return -1;
  }

  flux_t *h;
  if (!(h = flux_open(NULL, 0))) {
      std::cerr << "Error: 'priorisea score update ...'" << '\n'
                << "  could not connect to flux" << '\n'
                << UPDATE_USAGE_MSG << '\n'
                << UPDATE_TRY_HELP_MSG << std::endl;
      return -1;
  }

  std::string formula_name = "score";
  auto get_res = get_formula(formula_name);
  if(!get_res.ok) {
      std::cerr << "Error: 'priorisea score update ...'" << '\n'
                << "  could not get formula: " << get_res.message << '\n'
                << UPDATE_USAGE_MSG << '\n'
                << UPDATE_TRY_HELP_MSG << std::endl;
      return -1;
  }

  // TODO: support --new flag
  for (const auto &jobid_str : jobids) {
    flux_jobid_t jobid;
    if(flux_job_id_parse(jobid_str.c_str(), &jobid) < 0) {
        std::cerr << "Error: invalid job id: '" << jobid_str << "'" << std::endl;
        continue;
    }

    flux_future_t *f = flux_job_result(h, jobid, 0);
    if(flux_future_wait_for(f, 1) < 0) {
        std::cerr << "Error: job not ready (1): '" << jobid << "'" << std::endl;
        continue;
    }

    int result;
    double t_submit, t_run, t_cleanup;
    int waitstatus;
    bool exception;

    flux_job_result_get_unpack(
        f,
        "{s:i s:f s:f s:f s:i s:b}",
        "result", &result,
        "t_submit", &t_submit,
        "t_run", &t_run,
        "t_cleanup", &t_cleanup,
        "waitstatus", &waitstatus,
        "exception_occurred", &exception
    );
    flux_future_destroy(f);

    // if the job does not exist, it will raise an exception
    if(exception) {
        std::cerr << "Error: exception occurred: '" << jobid << "'" << std::endl;
        continue;
    }

    std::cout << "jobid: " << jobid << " result: " << result << " t_submit: " << t_submit << " t_run: " << t_run << " t_cleanup: " << t_cleanup << " waitstatus: " << waitstatus << " exception: " << exception << std::endl;

    std::map<std::string, double> vars_over;
    auto eval_res = eval_formula(get_res.formula_obj, vars_over);
    if (!eval_res.ok) {
        std::cerr << "Error: failed to evaluate formula" << std::endl;
        continue;
    }

    auto txn = flux_kvs_txn_create();
    if (!txn) {
        std::cerr << "Error: failed to create KVS transaction." << std::endl;
        continue;
    }
    if (flux_kvs_txn_put(txn, 0, ("priorisea.job." + std::to_string(jobid)).c_str(), std::to_string(eval_res.result).c_str()) < 0) {
        std::cerr << "Error: failed to store job score in KVS." << std::endl;
        continue;
    }
    auto commit_res = kvs_commit(txn);
    if (commit_res != KVS_ERR_SUCCESS) {
        std::cerr << "Error: failed to commit job score to KVS." << std::endl;
        continue;
    }

    std::cout << "Success: job score stored in KVS." << std::endl;
  }

  flux_close(h);
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
