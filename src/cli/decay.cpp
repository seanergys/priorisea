// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "decay.hpp"
#include "lib/decay.hpp"
#include "lib/error.hpp"
#include "util.hpp"
#include <bits/stdc++.h>
#include <iostream>
#include <jansson.h>
#include <string>

static void register_init_command(CLI::App *app) {
  auto *init_cmd = app->add_subcommand("init", "Initialize decay parameters");
  init_cmd->set_help_flag("-h,--help", "Show help for init command");
  init_cmd->callback([] {
    auto init_res = ps_db_decay_init();
    if (!ps_is_ok(init_res)) {
      return cli_error(init_res);
    }
  });
}

static void register_get_command(CLI::App *app) {
  auto *get_cmd = app->add_subcommand("get", "Get decay parameters");
  get_cmd->set_help_flag("-h,--help", "Show help for get command");
  struct options {
    bool json;
  };
  auto opts = std::make_shared<options>();
  get_cmd->add_flag("--json", opts->json, "Output JSON");
  get_cmd->callback([opts] {
    auto get_res = ps_db_decay_get();
    if (!ps_is_ok(get_res)) {
      return cli_error(get_res);
    }
    auto decay = ps_unwrap(get_res);
    if (opts->json) {
      auto obj = json_array();
      for (const auto &d : decay) {
        auto dec_one = json_object();
        json_object_set_new(dec_one, "duration", json_integer(d.duration));
        json_object_set_new(dec_one, "priority", json_real(d.priority));
        json_array_append(obj, dec_one);
      }
      char *dump = json_dumps(obj, JSON_INDENT(2));
      if (dump) {
        std::cout << dump << std::endl;
        free(dump);
      }
      json_decref(obj);
    } else {
      ps_print_table(decay, {{"Index",
                              [](auto &value, auto index) -> auto {
                                return std::to_string(index);
                              }},
                             {"Duration",
                              [](auto &value, auto index) -> auto {
                                return std::to_string(value.duration);
                              }},
                             {"Priority", [](auto &value, auto index) -> auto {
                                return std::to_string(value.priority);
                              }}});
    }
  });
}

static void register_set_command(CLI::App *app) {
  auto *set_cmd = app->add_subcommand("set", "Set decay parameters");
  set_cmd->footer("Example:\n  priorisea decay set 86400:1,604800:0.5 # set "
                  "decay to 1 for one day, 0.5 for the week after");
  set_cmd->set_help_flag("-h,--help", "Show help for set command");
  struct options {
    std::string expr;
  };
  auto opts = std::make_shared<options>();
  set_cmd->add_option("decay", opts->expr, "Decay expression")->required();
  set_cmd->callback([opts] {
    auto parse_res = ps_decay_from_str(opts->expr);
    if (!ps_is_ok(parse_res)) {
      return cli_error(parse_res);
    }
    auto decay = ps_unwrap(parse_res);
    auto store_res = ps_db_decay_set(decay);
    if (!ps_is_ok(store_res)) {
      return cli_error(store_res);
    }
  });
}

static void register_reset_command(CLI::App *app) {
  auto *reset_cmd =
      app->add_subcommand("reset", "Reset formulas to default values");
  reset_cmd->set_help_flag("-h,--help", "Show help for reset command");
  reset_cmd->callback([] {
    auto reset_res = ps_db_decay_reset();
    if (!ps_is_ok(reset_res)) {
      return cli_error(reset_res);
    }
  });
}

void register_decay_commands(CLI::App *app) {
  auto *decay_cmd = app->add_subcommand("decay", "Manage decay parameters");
  decay_cmd->require_subcommand(true);
  decay_cmd->set_help_flag("-h,--help", "Show help for decay commands");
  register_init_command(decay_cmd);
  register_get_command(decay_cmd);
  register_set_command(decay_cmd);
  register_reset_command(decay_cmd);
}
