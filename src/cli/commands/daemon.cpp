// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/daemon/daemon.hpp"
#include "src/cli/commands/daemon.hpp"
#include "src/cli/commands/database.hpp"
#include "src/lib/types.hpp"
#include <memory>
#include <string>
#include <unistd.h>

static void register_start_command(CLI::App *app) {
  auto *start_cmd = app->add_subcommand("start", "Start the daemon");
  start_cmd->set_help_flag("-h,--help", "Show help for start command");
  struct options {
    ps_daemon_config_t config;
    bool background;
  };
  auto opts = std::make_shared<options>();
  start_cmd->add_option("path", opts->config.database_path, "Path to database")
      ->default_val(DATABASE_PATH_DEFAULT);
  start_cmd
      ->add_option("-s,--seanerbus", opts->config.seanerbus_url,
                   "SeanerBUS endpoint")
      ->default_str(SEANERBUS_ENDPOINT_DEFAULT);
  start_cmd->add_flag("-b,--background", opts->background, "Run in background");
  start_cmd->callback([opts] {
    if (!opts->background) {
      ps_daemon_run(opts->config);
    } else {
      pid_t pid = fork();
      if (pid > 0) {
        std::cout << "Daemon started in background with PID " << pid
                  << std::endl;
        return;
      }
      setsid();
      ps_daemon_run(opts->config);
    }
  });
}

void register_daemon_commands(CLI::App *app) {
  auto *daemon_cmd = app->add_subcommand("daemon", "Manage the daemon");
  daemon_cmd->require_subcommand(true);
  daemon_cmd->set_help_flag("-h,--help", "Show help for daemon commands");
  register_start_command(daemon_cmd);
}
