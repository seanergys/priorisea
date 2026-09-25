// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "plugin.hpp"
#include "lib/flux.hpp"
#include "util.hpp"
#include <string>

static void register_seanerbus_get_command(CLI::App *app) {
  auto *get_cmd = app->add_subcommand("get", "Get Seanerbus connection info");
  get_cmd->set_help_flag("-h,--help", "Show help for get command");
  struct options {
    bool json;
  };
  auto opts = std::make_shared<options>();
  get_cmd->add_flag("--json", opts->json, "Output JSON");
  get_cmd->callback([opts] {
    auto get_res = ps_flux_plugin_get_seanerbus();
    if (!ps_is_ok(get_res)) {
      return cli_error(get_res);
    }
    auto seanerbus = ps_unwrap(get_res);

    if (opts->json) {
      auto obj = ps_seanerbus_to_json(seanerbus);
      char *dump = json_dumps(obj, JSON_INDENT(2) | JSON_ENCODE_ANY);
      if (dump) {
        std::cout << dump << std::endl;
        free(dump);
      }
      json_decref(obj);
    } else {
      auto seanerbus_vec = std::vector<ps_seanerbus_t>{seanerbus};
      ps_print_table_v(
          seanerbus_vec,
          {
              {"URL", [](auto &value, auto index) -> auto { return value; }},
          });
    }
  });
}

static void register_seanerbus_set_command(CLI::App *app) {
  auto *set_cmd = app->add_subcommand("set", "Set Seanerbus connection info");
  set_cmd->set_help_flag("-h,--help", "Show help for set command");
  struct options {
    std::string url;
  };
  auto opts = std::make_shared<options>();
  set_cmd->add_option("url", opts->url, "URL address of the Seanerbus server")
      ->required();
  set_cmd->callback([opts] {
    auto res = ps_flux_plugin_set_seanerbus(opts->url);
    if (!ps_is_ok(res)) {
      return cli_error(res);
    }
  });
}

static void register_seanerbus_command(CLI::App *app) {
  auto *seanerbus_cmd =
      app->add_subcommand("seanerbus", "Manage Seanerbus connection");
  seanerbus_cmd->require_subcommand(true);
  seanerbus_cmd->set_help_flag("-h,--help", "Show help for seanerbus command");
  register_seanerbus_get_command(seanerbus_cmd);
  register_seanerbus_set_command(seanerbus_cmd);
}

void register_plugin_commands(CLI::App *app) {
  auto *plugin_cmd = app->add_subcommand("plugin", "Manage Flux plugin");
  plugin_cmd->require_subcommand(true);
  plugin_cmd->set_help_flag("-h,--help", "Show help for plugin commands");
  register_seanerbus_command(plugin_cmd);
}
