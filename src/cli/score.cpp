// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "score.hpp"
#include "lib/error.hpp"
#include "lib/flux.hpp"
#include "lib/formula.hpp"
#include "lib/score.hpp"
#include "lib/types.hpp"
#include "util.hpp"
#include <flux/core.h>
#include <iostream>
#include <jansson.h>
#include <string>
#include <vector>

static void register_init_command(CLI::App *app) {
  auto *init_cmd =
      app->add_subcommand("init", "Initialize score database for priorisea");
  init_cmd->set_help_flag("-h,--help", "Show help for init command");
  init_cmd->callback([] {
    auto res = ps_db_score_init();
    if (!ps_is_ok(res)) {
      return cli_error(res);
    }
  });
}

static void register_update_command(CLI::App *app) {
  auto *update_cmd =
      app->add_subcommand("update", "Update jobs and users scores");
  update_cmd->footer(
      "Example:\n  priorisea score update --new f25GUqV1 # update scores for "
      "all new jobs and job f25GUqV1. This will also update scores for the "
      "users who ran these jobs");

  update_cmd->set_help_flag("-h,--help", "Show help for update command");
  struct options {
    bool new_f;
    std::vector<std::string> jobids;
  };
  auto opts = std::make_shared<options>();
  update_cmd->add_flag("--new", opts->new_f, "Update scores for all new jobs");
  update_cmd->add_option("jobids", opts->jobids, "Job IDs to retrieve");
  update_cmd->callback([opts] {
    auto get_formula_res = ps_db_formula_get("job_score");
    if (!ps_is_ok(get_formula_res)) {
      return cli_error(get_formula_res);
    }

    std::vector<ps_flux_job_info_t> jobs_to_update;
    if (opts->new_f) {
      auto get_t_inactive_res = ps_db_t_inactive_get();
      if (!ps_is_ok(get_t_inactive_res)) {
        return cli_error(get_t_inactive_res);
      }

      auto list_inactive_res =
          ps_flux_list_inactive_jobs(ps_unwrap(get_t_inactive_res), 10);
      if (!ps_is_ok(list_inactive_res)) {
        return cli_error(list_inactive_res);
      }

      for (auto &get_inactive_job_res : ps_unwrap(list_inactive_res).result) {
        if (ps_is_ok(get_inactive_job_res)) {
          jobs_to_update.push_back(ps_unwrap(get_inactive_job_res));
        }
      }

      if (!jobs_to_update.empty()) {
        auto store_t_inactive_res =
            ps_db_t_inactive_set(ps_unwrap(list_inactive_res).t_inactive_max);
        if (!ps_is_ok(store_t_inactive_res)) {
          return cli_error(store_t_inactive_res);
        }
      }
    }

    if (!opts->jobids.empty()) {
      auto list_res = ps_flux_list_jobs(opts->jobids);
      if (!ps_is_ok(list_res)) {
        return cli_error(list_res);
      }

      for (auto &get_job_res : ps_unwrap(list_res)) {
        if (!ps_is_ok(get_job_res.result)) {
          return cli_error(get_job_res.result);
        }
        jobs_to_update.push_back(ps_unwrap(get_job_res.result));
      }
    }

    // TODO: --new and specifying jobid can both find the
    // same job twice, thus updating it twice
    for (const auto &list_job : jobs_to_update) {
      auto update_res = ps_db_job_update(list_job, ps_unwrap(get_formula_res));
      if (!ps_is_ok(update_res)) {
        return cli_error(update_res);
      }
    }

    // Compute users scores
    auto update_users_res = ps_db_users_update();
    if (!ps_is_ok(update_users_res)) {
      return cli_error(update_users_res);
    }
  });
}

static void register_job_command(CLI::App *app) {
  auto *job_cmd = app->add_subcommand("job", "Manage job scores");
  job_cmd->footer(
      "Example:\n  priorisea score job f25GUqV1 # get score for job f25GUqV1");
  job_cmd->set_help_flag("-h,--help", "Show help for job command");
  struct options {
    bool json;
    bool all;
    std::vector<std::string> jobids;
  };
  auto opts = std::make_shared<options>();
  job_cmd->add_flag("--json", opts->json, "Output in JSON format");
  job_cmd->add_flag("--all", opts->all, "Output all users");
  job_cmd->add_option("jobids", opts->jobids, "Job IDs to retrieve");
  job_cmd->callback([opts] {
    std::vector<ps_job_t> jobs;
    if (opts->all) {
      auto list_res = ps_db_jobs_list();
      if (!ps_is_ok(list_res)) {
        return cli_error(list_res);
      }

      for (const auto &job : ps_unwrap(list_res)) {
        if (!ps_is_ok(job.result)) {
          return cli_error(job.result);
        }
        jobs.push_back(ps_unwrap(job.result));
      }
    } else {
      auto get_res = ps_db_jobs_get(opts->jobids);
      if (!ps_is_ok(get_res)) {
        return cli_error(get_res);
      }

      for (const auto &get_job_res : ps_unwrap(get_res)) {
        if (!ps_is_ok(get_job_res.result)) {
          return cli_error(get_job_res.result);
        }
        jobs.push_back(ps_unwrap(get_job_res.result));
      }
    }

    if (opts->json) {
      auto obj = json_object();
      for (const auto &f : jobs) {
        json_object_set_new(obj, f.jobid.c_str(), json_integer(f.score));
      }
      std::cout << json_dumps(obj, JSON_INDENT(2)) << std::endl;
      json_decref(obj);
    } else {
      ps_print_table(
          jobs, {{"Job Id",
                  [](auto &value, auto index) -> auto { return value.jobid; }},
                 {"Score", [](auto &value, auto index) -> auto {
                    return std::to_string(value.score);
                  }}});
    }
  });
}

static void register_user_command(CLI::App *app) {
  auto *user_cmd = app->add_subcommand("user", "Manage user scores");
  user_cmd->footer(
      "Example:\n  priorisea score user 1001 # get score for user 1001");
  user_cmd->set_help_flag("-h,--help", "Show help for user command");
  struct options {
    bool json;
    bool all;
    std::vector<int> userids;
  };
  auto opts = std::make_shared<options>();
  user_cmd->add_flag("--json", opts->json, "Output in JSON format");
  user_cmd->add_flag("--all", opts->all, "Output all users");
  user_cmd->add_option("userids", opts->userids, "User IDs to retrieve");
  user_cmd->callback([opts] {
    std::vector<ps_user_t> users;
    if (opts->all) {
      auto list_res = ps_db_users_list();
      if (!ps_is_ok(list_res)) {
        return cli_error(list_res);
      }

      for (const auto &user : ps_unwrap(list_res)) {
        if (!ps_is_ok(user.result)) {
          return cli_error(user.result);
        }
        users.push_back(ps_unwrap(user.result));
      }
    } else {
      auto get_res = ps_db_users_get(opts->userids);
      if (!ps_is_ok(get_res)) {
        return cli_error(get_res);
      }

      for (const auto &user : ps_unwrap(get_res)) {
        if (!ps_is_ok(user.result)) {
          return cli_error(user.result);
        }

        users.push_back(ps_unwrap(user.result));
      }
    }

    if (opts->json) {
      auto obj = json_object();
      for (const auto &f : users) {
        json_object_set_new(obj, std::to_string(f.userid).c_str(),
                            json_integer(f.score));
      }
      char *json_str = json_dumps(obj, JSON_INDENT(2));
      if (json_str) {
        std::cout << json_str << std::endl;
        free(json_str);
      }
      json_decref(obj);
    } else {
      ps_print_table(users, {{"UserId",
                              [](auto &value, auto index) -> auto {
                                return std::to_string(value.userid);
                              }},
                             {"Score", [](auto &value, auto index) -> auto {
                                return std::to_string(value.score);
                              }}});
    }
  });
}

void register_score_commands(CLI::App *app) {
  auto *score_cmd = app->add_subcommand("score", "Manage priority scores");
  score_cmd->require_subcommand(true);
  score_cmd->set_help_flag("-h,--help", "Show help for score commands");
  register_user_command(score_cmd);
  register_job_command(score_cmd);
  register_update_command(score_cmd);
  register_init_command(score_cmd);
}
