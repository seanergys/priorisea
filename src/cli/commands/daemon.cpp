#include "src/daemon/daemon.hpp"
#include "src/cli/commands/daemon.hpp"
#include "src/cli/commands/database.hpp"
#include "src/cli/endpoint/flux/info.hpp"
#include "src/cli/endpoint/info.hpp"
#include "src/cli/util.hpp"
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
      ->default_val(SEANERBUS_ENDPOINT_DEFAULT);
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

static void register_debug_command(CLI::App *app) {
  auto *debug_cmd = app->add_subcommand("debug", "Debug print for the daemon");
  debug_cmd->set_help_flag("-h,--help", "Show help for debug command");
  debug_cmd->callback([] {
    auto info_res = ps_cli_endpoint_info_get();
    if (!ps_is_ok(info_res)) {
      ps_exit_error(info_res);
    }
    auto info = ps_unwrap(info_res);
    std::vector<ps_cli_endpoint_info_get_t> vec = {info};
    ps_print_table_v(vec, {{"Version",
                            [](auto &value, auto index) -> auto {
                              return std::to_string(value.version);
                            }},
                           {"t_inactive",
                            [](auto &value, auto index) -> auto {
                              return std::to_string(value.info.t_inactive);
                            }},
                           {"user_period", [](auto &value, auto index) -> auto {
                              return std::to_string(value.info.user_period);
                            }}});
  });
}

void register_daemon_commands(CLI::App *app) {
  auto *daemon_cmd = app->add_subcommand("daemon", "Manage the daemon");
  daemon_cmd->require_subcommand(true);
  daemon_cmd->set_help_flag("-h,--help", "Show help for daemon commands");
  register_start_command(daemon_cmd);
  register_debug_command(daemon_cmd);
}
