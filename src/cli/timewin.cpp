// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "timewin.hpp"
#include "lib/error.hpp"
#include "lib/timewin.hpp"
#include "lib/types.hpp"
#include "util.hpp"
#include <cstdlib>
#include <flux/core.h>
#include <iostream>
#include <jansson.h>
#include <optional>
#include <string>
#include <vector>

static void print_timewins_json(const std::vector<ps_timewin_t> &timewins) {
  auto obj = json_object();
  for (const auto &timewin : timewins) {
    auto expr = json_object();
    json_object_set_new(expr, "cron", json_string(timewin.cron.c_str()));
    json_object_set_new(expr, "duration", json_integer(timewin.duration));
    json_object_set_new(expr, "priority", json_real(timewin.priority));
    json_object_set_new(obj, timewin.name.c_str(), expr);
  }
  char *dump = json_dumps(obj, JSON_INDENT(2));
  if (dump) {
    std::cout << dump << std::endl;
    free(dump);
  }
  json_decref(obj);
}

static void print_timewins_table(const std::vector<ps_timewin_t> &timewins) {
  ps_print_table(
      timewins,
      {{"Name", [](auto &value, auto index) -> auto { return value.name; }},
       {"Cron", [](auto &value, auto index) -> auto { return value.cron; }},
       {"Duration",
        [](auto &value, auto index) -> auto {
          return std::to_string(value.duration);
        }},
       {"Priority", [](auto &value, auto index) -> auto {
          return std::to_string(value.priority);
        }}});
}

////

static void register_get_command(CLI::App *app) {
  auto *get_cmd = app->add_subcommand("get", "Get a time window");
  get_cmd->set_help_flag("-h,--help", "Show help for get command");
  struct options {
    std::vector<std::string> timewins_names;
    bool json = false;
  };
  auto opts = std::make_shared<options>();
  get_cmd
      ->add_option("names", opts->timewins_names,
                   "Names of the time windows to get")
      ->required();
  get_cmd->add_flag("-j,--json", opts->json, "Output in JSON format");
  get_cmd->callback([opts] {
    auto get_res = ps_db_timewins_get(opts->timewins_names);
    if (!ps_is_ok(get_res)) {
      return cli_error(get_res);
    }
    std::vector<ps_timewin_t> timewins;
    for (const auto &get_timewin_res : ps_unwrap(get_res)) {
      if (!ps_is_ok(get_timewin_res.result)) {
        return cli_error(get_timewin_res.result);
      }
      timewins.push_back(ps_unwrap(get_timewin_res.result));
    }
    if (opts->json) {
      print_timewins_json(timewins);
    } else {
      print_timewins_table(timewins);
    }
  });
}

static void register_add_command(CLI::App *app) {
  auto *add_cmd = app->add_subcommand("add", "Add a time window");
  add_cmd->footer("Example:\n  priorisea timewin add monday -p 10 -c \"0 0 0 * "
                  "* 1\" -d 86400 # on monday get 10 points");
  add_cmd->set_help_flag("-h,--help", "Show help for add command");
  auto opts = std::make_shared<ps_timewin_t>();
  add_cmd->add_option("name", opts->name, "Name of the time window")
      ->required();
  add_cmd
      ->add_option("-p,--priority", opts->priority,
                   "Priority of the time window")
      ->required();
  add_cmd
      ->add_option("-c,--cron", opts->cron,
                   "Cron expression for the time window")
      ->required();
  add_cmd
      ->add_option("-d,--duration", opts->duration,
                   "Duration of the time window")
      ->required();
  add_cmd->callback([opts] {
    auto validate_res = ps_validate_timewin(*opts);
    if (!ps_is_ok(validate_res)) {
      return cli_error(validate_res);
    }
    auto store_res = ps_db_timewin_set(*opts);
    if (!ps_is_ok(store_res)) {
      return cli_error(store_res);
    }
  });
}

static void register_edit_command(CLI::App *app) {
  auto *edit_cmd = app->add_subcommand("edit", "Edit a time window");
  edit_cmd->footer("Example:\n  priorisea timewin edit monday -c \"0 0 0 * * "
                   "2\" # change cron to run every tuesday");
  edit_cmd->set_help_flag("-h,--help", "Show help for edit command");
  struct options {
    std::string name;
    std::optional<std::string> cron;
    std::optional<int> priority;
    std::optional<int> duration;
  };
  auto opts = std::make_shared<options>();
  edit_cmd->add_option("name", opts->name, "Name of the time window")
      ->required();
  edit_cmd->add_option("-p,--priority", opts->priority,
                       "Priority of the time window");
  edit_cmd->add_option("-c,--cron", opts->cron,
                       "Cron expression for the time window");
  edit_cmd->add_option("-d,--duration", opts->duration,
                       "Duration of the time window");
  edit_cmd->callback([opts] {
    auto get_res = ps_db_timewin_get(opts->name);
    if (!ps_is_ok(get_res)) {
      return cli_error(get_res);
    }
    auto timewin = ps_unwrap(get_res);
    if (opts->cron.has_value()) {
      timewin.cron = opts->cron.value();
    }
    if (opts->duration.has_value()) {
      timewin.duration = opts->duration.value();
    }
    if (opts->priority.has_value()) {
      timewin.priority = opts->priority.value();
    }

    auto validate_res = ps_validate_timewin(timewin);
    if (!ps_is_ok(validate_res)) {
      return cli_error(validate_res);
    }

    auto store_res = ps_db_timewin_set(timewin);
    if (!ps_is_ok(store_res)) {
      return cli_error(store_res);
    }
  });
}

static void register_del_command(CLI::App *app) {
  auto *del_cmd = app->add_subcommand("delete", "Delete a time window");
  del_cmd->set_help_flag("-h,--help", "Show help for delete command");
  struct options {
    std::vector<std::string> timewins_names;
  };
  auto opts = std::make_shared<options>();
  del_cmd
      ->add_option("names", opts->timewins_names,
                   "Names of the time windows to delete")
      ->required();
  del_cmd->callback([opts] {
    for (const auto &timewin_name : opts->timewins_names) {
      auto del_res = ps_db_timewin_del(timewin_name);
      if (!ps_is_ok(del_res)) {
        return cli_error(del_res);
      }
    }
  });
}

static void register_list_command(CLI::App *app) {
  auto *list_cmd = app->add_subcommand("list", "List configured time windows");
  list_cmd->set_help_flag("-h,--help", "Show help for list command");
  struct options {
    bool json;
  };
  auto opts = std::make_shared<options>();
  list_cmd->add_flag("--json", opts->json, "Output in JSON format");

  list_cmd->callback([opts] {
    auto list_res = ps_db_timewins_list();
    if (!ps_is_ok(list_res)) {
      return cli_error(list_res);
    }
    std::vector<ps_timewin_t> timewins;
    for (const auto &res : ps_unwrap(list_res)) {
      if (!ps_is_ok(res.result)) {
        return cli_error(list_res);
      }
      timewins.push_back(ps_unwrap(res.result));
    }
    if (opts->json) {
      print_timewins_json(timewins);
    } else {
      print_timewins_table(timewins);
    }
  });
}

void register_timewin_commands(CLI::App *app) {
  auto *timewin_cmd =
      app->add_subcommand("timewin", "Manage priority time windows");
  timewin_cmd->require_subcommand(true);
  timewin_cmd->set_help_flag("-h,--help", "Show help for timewin commands");
  register_list_command(timewin_cmd);
  register_get_command(timewin_cmd);
  register_add_command(timewin_cmd);
  register_del_command(timewin_cmd);
  register_edit_command(timewin_cmd);
}
