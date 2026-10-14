// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/cli/endpoint/flux/decay.hpp"
#include "src/cli/commands/decay.hpp"
#include "src/cli/util.hpp"
#include "src/lib/decay.hpp"
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <jansson.h>

static void print_decay(const ps_decay_t &decay, bool json) {
  if (json) {
    auto obj = ps_decay_to_json(decay);
    char *dump = json_dumps(obj, JSON_INDENT(2));
    if (dump) {
      std::cout << dump << std::endl;
      free(dump);
    }
    json_decref(obj);
  } else {
    ps_print_table_v(decay, {{"Index",
                              [](auto &value, auto index) -> auto {
                                return std::to_string(index);
                              }},
                             {"Duration",
                              [](auto &value, auto index) -> auto {
                                return std::to_string(value.duration);
                              }},
                             {"Weight", [](auto &value, auto index) -> auto {
                                return std::to_string(value.weight);
                              }}});
  }
}

static void register_get_command(CLI::App *app) {
  auto *get_cmd = app->add_subcommand("get", "Get decay");
  get_cmd->set_help_flag("-h,--help", "Show help for get command");
  bool json;
  get_cmd->add_flag("-j,--json", json, "Output JSON");
  get_cmd->callback([json] {
    auto get_res = ps_cli_endpoint_flux_decay_get();
    if (!ps_is_ok(get_res)) {
      ps_exit_error(get_res);
    }
    auto decay = ps_unwrap(get_res);
    print_decay(decay, json);
  });
}

static void register_reset_command(CLI::App *app) {
  auto *reset_cmd = app->add_subcommand("reset", "Reset decay");
  reset_cmd->set_help_flag("-h,--help", "Show help for reset command");
  reset_cmd->callback([] {
    auto reset_res = ps_cli_endpoint_flux_decay_reset();
    if (!ps_is_ok(reset_res)) {
      ps_exit_error(reset_res);
    }
  });
}

void register_decay_commands(CLI::App *app) {
  auto *decay_cmd = app->add_subcommand("decay", "Manage the decay");
  decay_cmd->require_subcommand(true);
  decay_cmd->set_help_flag("-h,--help", "Show help for decay commands");
  register_get_command(decay_cmd);
  register_reset_command(decay_cmd);
}
