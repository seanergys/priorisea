// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "decay.hpp"
#include "lib/decay.hpp"
#include "lib/error.hpp"
#include "util.hpp"
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
      auto obj = ps_decay_to_json(decay);
      char *dump = json_dumps(obj, JSON_INDENT(2));
      if (dump) {
        std::cout << dump << std::endl;
        free(dump);
      }
      json_decref(obj);
    } else {
      ps_print_table_v(decay,
                       {{"Index",
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
  set_cmd->footer(
      "Example:\n  priorisea decay set \"3600:1,7200:0.5\" # string format\n  "
      "priorisea decay set "
      "'[{\"duration\":86400,\"priority\":1},{\"duration\":604800,"
      "\"priority\":0.5}]' --json # JSON format");
  set_cmd->set_help_flag("-h,--help", "Show help for set command");
  struct options {
    std::string expr;
    bool json;
  };
  auto opts = std::make_shared<options>();
  set_cmd->add_option("decay", opts->expr, "Decay expression")->required();
  set_cmd->add_flag("--json", opts->json, "Parse expression as JSON");
  set_cmd->callback([opts] {
    ps_decay_t decay;
    if (opts->json) {
      json_error_t json_err;
      auto *json_decay = json_loads(opts->expr.c_str(), 0, &json_err);
      if (!json_decay) {
        return cli_error(ps_error(ps_json_unpack_err_t{.dumps = opts->expr}));
      }
      auto decay_res = ps_decay_from_json(json_decay);
      json_decref(json_decay);
      if (!ps_is_ok(decay_res)) {
        return cli_error(decay_res);
      }
      decay = ps_unwrap(decay_res);
    } else {
      auto decay_res = ps_decay_from_string(opts->expr);
      if (!ps_is_ok(decay_res)) {
        return cli_error(decay_res);
      }
      decay = ps_unwrap(decay_res);
    }
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
