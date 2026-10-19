// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/cli/commands/plugin.hpp"
#include "src/cli/endpoint/flux/plugin.hpp"
#include "src/cli/util.hpp"
#include "src/lib/error.hpp"
#include <jansson.h>
#include <string>
#include <sys/types.h>

static void print_scores(const std::map<uid_t, double> &scores, bool json) {
  if (json) {
    auto obj = json_object();
    for (const auto &[user, score] : scores) {
      json_object_set(obj, std::to_string(user).c_str(), json_real(score));
    }
    char *dump = json_dumps(obj, JSON_INDENT(2));
    if (dump) {
      std::cout << dump << std::endl;
      free(dump);
    }
    json_decref(obj);
    return;
  }
  struct score_struct {
    uid_t uid;
    double score;
  };
  std::vector<score_struct> scores_vec;
  for (const auto &[user, score] : scores) {
    scores_vec.push_back({.uid = user, .score = score});
  }
  ps_print_table_v(
      scores_vec,
      {{"UID", [](auto &u, auto) { return std::to_string(u.uid); }},
       {"Score", [](auto &u, auto) { return std::to_string(u.score); }}});
}

static void register_sync_command(CLI::App *app) {
  auto *sync_cmd =
      app->add_subcommand("sync", "Sync the plugin with the database");
  sync_cmd->set_help_flag("-h,--help", "Show help for sync command");
  sync_cmd->callback([] {
    auto sync_res = ps_cli_endpoint_flux_plugin_sync();
    if (!ps_is_ok(sync_res)) {
      ps_exit_error(sync_res);
    }
  });
}

static void register_debug_command(CLI::App *app) {
  auto *debug_cmd = app->add_subcommand("debug", "Debug print for plugin");
  debug_cmd->set_help_flag("-h,--help", "Show help for debug command");
  bool json;
  debug_cmd->add_flag("-j,--json", json, "Output JSON");
  debug_cmd->callback([json] {
    auto debug_res = ps_cli_endpoint_flux_plugin_debug();
    if (!ps_is_ok(debug_res)) {
      ps_exit_error(debug_res);
    }
    auto debug = ps_unwrap(debug_res);
    print_formula(debug.formula, json);
    print_scores(debug.scores, json);
  });
}

void register_plugin_commands(CLI::App *app) {
  auto *plugin_cmd = app->add_subcommand("plugin", "Manage the plugin");
  plugin_cmd->require_subcommand(true);
  plugin_cmd->set_help_flag("-h,--help", "Show help for plugin commands");
  register_sync_command(plugin_cmd);
  register_debug_command(plugin_cmd);
}
