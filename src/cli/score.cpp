// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "score.hpp"
#include "CLI/CLI.hpp"
#include "lib/config.hpp"
#include "lib/error.hpp"
#include "lib/flux.hpp"
#include "lib/formula.hpp"
#include "lib/score.hpp"
#include "lib/timewin.hpp"
#include "lib/types.hpp"
#include "lib/util.hpp"
#include "util.hpp"
#include <climits>
#include <cstdlib>
#include <errno.h>
#include <flux/core.h>
#include <flux/core/job.h>
#include <iostream>
#include <jansson.h>
#include <string>
#include <sys/types.h>
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

static void register_update_command(CLI::App *app,
                                    const std::string &config_path) {
  auto *update_cmd =
      app->add_subcommand("update", "Update jobs and users scores");
  update_cmd->footer(
      "Example:\n  priorisea score update --new f25GUqV1 # update scores for "
      "all new jobs and job f25GUqV1. This will also update scores for the "
      "users who ran these jobs");

  update_cmd->set_help_flag("-h,--help", "Show help for update command");
  struct options {
    bool new_f;
    std::vector<std::string> jobids_str;
  };
  auto opts = std::make_shared<options>();
  update_cmd->add_flag("--new", opts->new_f, "Update scores for all new jobs");
  update_cmd->add_option("jobids", opts->jobids_str, "Job IDs to retrieve");
  update_cmd->callback([opts, &config_path] {
    auto config_res = ps_config_load(config_path);
    if (!ps_is_ok(config_res)) {
      return cli_error(config_res);
    }
    auto config = ps_unwrap(config_res);

    auto get_formula_res = ps_db_formula_get("job_score");
    if (!ps_is_ok(get_formula_res)) {
      return cli_error(get_formula_res);
    }

    std::vector<ps_flux_job_info_t> jobs_to_update;

    // retrieve new jobs
    if (opts->new_f) {
      // retrieve the last inactive time threshold
      auto get_t_inactive_res = ps_db_t_inactive_get();
      if (!ps_is_ok(get_t_inactive_res)) {
        return cli_error(get_t_inactive_res);
      }

      // retrieve new inactive jobs since the last threshold
      auto list_inactive_res =
          ps_flux_list_inactive_jobs(ps_unwrap(get_t_inactive_res), INT_MAX);
      if (!ps_is_ok(list_inactive_res)) {
        return cli_error(list_inactive_res);
      }

      for (auto &get_inactive_job_res : ps_unwrap(list_inactive_res).result) {
        if (ps_is_ok(get_inactive_job_res)) {
          jobs_to_update.push_back(ps_unwrap(get_inactive_job_res));
        }
      }

      // store the new inactive time threshold
      // TODO: if there is an error afterward, the jobs will not be considered
      // "new" but will not be updated
      if (!jobs_to_update.empty()) {
        auto store_t_inactive_res =
            ps_db_t_inactive_set(ps_unwrap(list_inactive_res).t_inactive_max);
        if (!ps_is_ok(store_t_inactive_res)) {
          return cli_error(store_t_inactive_res);
        }
      }
    }

    // retrieve the specified jobs to update
    if (!opts->jobids_str.empty()) {
      std::vector<flux_jobid_t> jobids;
      for (const auto &jobid_str : opts->jobids_str) {
        flux_jobid_t jobid;
        if (flux_job_id_parse(jobid_str.c_str(), &jobid) < 0) {
          return cli_error(ps_error(ps_flux_jobid_parse_err_t{
              .jobid = jobid_str, .msg = strerror(errno)}));
        }
        jobids.push_back(jobid);
      }
      auto list_res = ps_flux_list_jobs(jobids);
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

    // retrieve all timewins
    std::vector<ps_timewin_t> timewins;
    auto get_timewins_res = ps_db_timewins_list();
    // if no timewin is set, this will return an error
    // TODO: differentiate no data and error
    if (ps_is_ok(get_timewins_res)) {
      auto get_timewins = ps_unwrap(get_timewins_res);
      for (const auto &get_timewin : get_timewins) {
        if (!ps_is_ok(get_timewin.result)) {
          return cli_error(get_timewin.result);
        }
        timewins.push_back(ps_unwrap(get_timewin.result));
      }
    }

    // TODO: --new and specifying jobid can both find the
    // same job twice, thus updating it twice
    for (const auto &list_job : jobs_to_update) {
      auto update_res = ps_db_job_update(list_job, ps_unwrap(get_formula_res),
                                         timewins, config.seanerbus);
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
    int limit;
    std::vector<std::string> jobids_str;
    std::string id_format;
  };
  auto opts = std::make_shared<options>();
  job_cmd->add_flag("--json", opts->json, "Output in JSON format");
  job_cmd->add_flag("--all", opts->all, "Output all users");
  job_cmd
      ->add_option("--limit,-n", opts->limit,
                   "Return the N most recent jobs, ranked by t_inactive")
      ->default_val(0);
  job_cmd->add_option("jobids", opts->jobids_str, "Job IDs to retrieve");
  job_cmd->add_option("--id-format", opts->id_format, "Job IDs output format")
      ->check(CLI::IsMember(
          {"dec", "hex", "kvs", "dothex", "words", "f58", "f58plain"}))
      ->default_val("dec");
  job_cmd->callback([opts] {
    std::vector<ps_job_t> jobs;
    if (opts->limit > 0) {
      auto list_res = ps_db_jobs_list_ranked((size_t)opts->limit);
      if (!ps_is_ok(list_res)) {
        return cli_error(list_res);
      }

      for (const auto &job : ps_unwrap(list_res)) {
        if (!ps_is_ok(job.result)) {
          continue;
        }
        jobs.push_back(ps_unwrap(job.result));
      }
    } else if (opts->all) {
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
      std::vector<flux_jobid_t> jobids;
      for (const auto &jobid_str : opts->jobids_str) {
        flux_jobid_t jobid;
        if (flux_job_id_parse(jobid_str.c_str(), &jobid) < 0) {
          return cli_error(ps_error(ps_flux_jobid_parse_err_t{
              .jobid = jobid_str, .msg = strerror(errno)}));
        }
        jobids.push_back(jobid);
      }
      auto get_res = ps_db_jobs_get(jobids);
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
      auto arr = json_array();
      for (const auto &job : jobs) {
        auto job_json = ps_job_to_json(job);
        if (opts->id_format != "dec") {
          char buf[256];
          flux_job_id_encode(job.jobid, opts->id_format.c_str(), buf,
                             sizeof(buf));
          auto jobid_json = json_string(buf);
          json_object_set(job_json, "jobid", jobid_json);
        }
        json_array_append_new(arr, job_json);
      }
      std::cout << json_dumps(arr, JSON_INDENT(2)) << std::endl;
      json_decref(arr);
    } else {
      ps_print_table_v(
          jobs, {{"Job Id",
                  [&opts](auto &value, auto index) -> auto {
                    if (opts->id_format == "dec") {
                      return std::to_string(value.jobid);
                    }
                    char buf[256];
                    flux_job_id_encode(value.jobid, opts->id_format.c_str(),
                                       buf, sizeof(buf));
                    return std::string(buf);
                  }},
                 {"Score",
                  [](auto &value, auto index) -> auto {
                    return std::to_string(value.score);
                  }},
                 {"t_inactive", [](auto &value, auto index) -> auto {
                    return std::to_string(value.t_inactive);
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
    std::vector<std::string> users;
  };
  auto opts = std::make_shared<options>();
  user_cmd->add_flag("--json", opts->json, "Output in JSON format");
  user_cmd->add_flag("--all", opts->all, "Output all users");
  user_cmd->add_option("userids", opts->users, "User names or IDs to retrieve");
  user_cmd->callback([opts] {
    typedef struct {
      ps_user_t user;
      std::string username;
    } user_info_t;
    std::vector<user_info_t> users;
    if (opts->all) {
      auto list_res = ps_db_users_list();
      if (!ps_is_ok(list_res)) {
        return cli_error(list_res);
      }

      for (const auto &user : ps_unwrap(list_res)) {
        if (!ps_is_ok(user.result)) {
          return cli_error(user.result);
        }
        users.push_back({.user = ps_unwrap(user.result), .username = ""});
      }
    } else {
      std::vector<uid_t> userids;
      for (const auto &userid : opts->users) {
        uid_t uid;
        if (parse_uid(userid, uid)) {
          userids.push_back(uid);
          continue;
        }
        if (uid_from_username(userid, uid)) {
          userids.push_back(uid);
          continue;
        }
        return cli_error(ps_error(ps_user_not_found_err_t{userid}));
      }
      auto get_res = ps_db_users_get(userids);
      if (!ps_is_ok(get_res)) {
        return cli_error(get_res);
      }

      for (const auto &user : ps_unwrap(get_res)) {
        if (!ps_is_ok(user.result)) {
          return cli_error(user.result);
        }

        users.push_back({.user = ps_unwrap(user.result), .username = ""});
      }
    }

    for (auto &user : users) {
      std::string username;
      if (username_from_uid(user.user.userid, username)) {
        user.username = username;
      }
    }

    if (opts->json) {
      auto arr = json_array();
      for (const auto &user : users) {
        auto user_json = json_object();
        json_object_set_new(user_json, "user",
                            json_string(user.username.c_str()));
        json_object_update(user_json, ps_user_to_json(user.user));
        json_array_append_new(arr, user_json);
      }
      char *json_str = json_dumps(arr, JSON_INDENT(2));
      if (json_str) {
        std::cout << json_str << std::endl;
        free(json_str);
      }
      json_decref(arr);
    } else {
      ps_print_table_v(users, {{"User",
                                [](auto &value, auto index) -> auto {
                                  return value.username;
                                }},
                               {"UserId",
                                [](auto &value, auto index) -> auto {
                                  return std::to_string(value.user.userid);
                                }},
                               {"Score", [](auto &value, auto index) -> auto {
                                  return std::to_string(value.user.score);
                                }}});
    }
  });
}

void register_score_commands(CLI::App *app, const std::string &config_path) {
  auto *score_cmd = app->add_subcommand("score", "Manage priority scores");
  score_cmd->require_subcommand(true);
  score_cmd->set_help_flag("-h,--help", "Show help for score commands");
  register_user_command(score_cmd);
  register_job_command(score_cmd);
  register_update_command(score_cmd, config_path);
  register_init_command(score_cmd);
}
