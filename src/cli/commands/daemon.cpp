#include "src/daemon/daemon.hpp"
#include "src/cli/commands/daemon.hpp"
#include "src/cli/commands/database.hpp"
#include "src/cli/endpoint/flux/info.hpp"
#include "src/cli/endpoint/info.hpp"
#include "src/cli/util.hpp"
#include "src/lib/info.hpp"
#include "src/lib/types.hpp"
#include <jansson.h>
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
        std::cout << pid << std::endl;
        return;
      }
      setsid();
      ps_daemon_run(opts->config);
    }
  });
}

static void print_info(const ps_cli_endpoint_info_get_t &info, bool json) {
  if (json) {
    auto *obj = json_object();
    json_object_set_new(obj, "version", json_integer(info.version));
    json_object_set_new(obj, "seanerbus_url",
                        json_string(info.seanerbus_url.c_str()));
    json_object_set_new(obj, "info", ps_db_info_to_json(info.info));
    char *dump = json_dumps(obj, JSON_INDENT(2));
    if (dump) {
      std::cout << dump << std::endl;
      free(dump);
    }
    json_decref(obj);
    return;
  }
  std::vector<ps_cli_endpoint_info_get_t> vec = {info};
  ps_print_table_v(vec, {{"Version",
                          [](auto &value, auto index) -> auto {
                            return std::to_string(value.version);
                          }},
                         {"Seanerbus URL",
                          [](auto &value, auto index) -> auto {
                            return value.seanerbus_url;
                          }},
                         {"t_inactive",
                          [](auto &value, auto index) -> auto {
                            return std::to_string(value.info.t_inactive);
                          }},
                         {"user_period", [](auto &value, auto index) -> auto {
                            return std::to_string(value.info.user_period);
                          }}});
}

static void register_debug_command(CLI::App *app) {
  auto *debug_cmd = app->add_subcommand("debug", "Debug print for the daemon");
  debug_cmd->set_help_flag("-h,--help", "Show help for debug command");
  struct options {
    bool json;
  };
  auto opts = std::make_shared<options>();
  debug_cmd->add_flag("-j,--json", opts->json, "Output JSON");
  debug_cmd->callback([opts] {
    auto info_res = ps_cli_endpoint_info_get();
    if (!ps_is_ok(info_res)) {
      ps_exit_error(info_res);
    }
    print_info(ps_unwrap(info_res), opts->json);
  });
}

static void register_update_user_period_command(CLI::App *app) {
  auto *uup_cmd =
      app->add_subcommand("update-user-period", "Update the user period");
  uup_cmd->set_help_flag("-h,--help",
                         "Show help for update-user-period command");
  struct options {
    double user_period;
  };
  auto opts = std::make_shared<options>();
  uup_cmd
      ->add_option("user_period", opts->user_period,
                   "Max. age (seconds) of jobs when computing users score")
      ->required();
  uup_cmd->callback([opts] {
    auto update_res =
        ps_cli_endpoint_flux_user_period_update(opts->user_period);
    if (!ps_is_ok(update_res)) {
      ps_exit_error(update_res);
    }
  });
}
void register_daemon_commands(CLI::App *app) {
  auto *daemon_cmd = app->add_subcommand("daemon", "Manage the daemon");
  daemon_cmd->require_subcommand(true);
  daemon_cmd->set_help_flag("-h,--help", "Show help for daemon commands");
  register_start_command(daemon_cmd);
  register_debug_command(daemon_cmd);
  register_update_user_period_command(daemon_cmd);
}
