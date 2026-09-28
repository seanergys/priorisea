// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "config.hpp"
#include "lib/config.hpp"
#include "lib/types.hpp"
#include "util.hpp"
#include <string>

static void register_get_command(CLI::App *app,
                                 const std::string &config_path) {
  auto *get_cmd = app->add_subcommand("get", "Get config");
  get_cmd->set_help_flag("-h,--help", "Show help for get command");
  struct options {
    bool json;
  };
  auto opts = std::make_shared<options>();
  get_cmd->add_flag("--json", opts->json, "Output JSON");
  get_cmd->callback([opts, &config_path] {
    auto config_res = ps_config_load(config_path);
    if (!ps_is_ok(config_res)) {
      return cli_error(config_res);
    }
    auto config = ps_unwrap(config_res);
    if (opts->json) {
      auto *json = ps_config_to_json(config);
      char *str = json_dumps(json, JSON_INDENT(2));
      std::cout << str << std::endl;
      free(str);
      json_decref(json);
    } else {
      auto config_vec = std::vector<ps_config_t>{config};
      ps_print_table_h(config_vec, {
                                       {"Seanerbus URL",
                                        [](auto &value, auto index) -> auto {
                                          return value.seanerbus;
                                        }},
                                   });
    }
  });
}

static void register_export_command(CLI::App *app,
                                    const std::string &config_path) {
  auto *export_cmd = app->add_subcommand("export", "Export config");
  export_cmd->set_help_flag("-h,--help", "Show help for export command");
  struct options {
    std::string path;
  };
  auto opts = std::make_shared<options>();
  export_cmd->add_option("path", opts->path, "Output path")->required(true);
  export_cmd->callback([opts, &config_path] {
    auto config_res = ps_config_load(config_path);
    if (!ps_is_ok(config_res)) {
      return cli_error(config_res);
    }
    auto config = ps_unwrap(config_res);
    auto *json = ps_config_to_json(config);
    char *str = json_dumps(json, JSON_INDENT(2));
    std::ofstream out(opts->path);
    out << str;
    free(str);
    json_decref(json);
  });
}

void register_config_commands(CLI::App *app, const std::string &config_path) {
  auto *config_cmd = app->add_subcommand("config", "Manage configuration");
  config_cmd->require_subcommand(true);
  config_cmd->set_help_flag("-h,--help", "Show help for config commands");
  register_get_command(config_cmd, config_path);
  register_export_command(config_cmd, config_path);
}
