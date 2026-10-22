#include "src/cli/commands/database.hpp"
#include "src/database/database.hpp"
#include "src/lib/error.hpp"
#include <memory>
#include <string>

static inline double USER_PERIOD_DEFAULT = 90 * 24 * 60 * 60;

static void register_create_command(CLI::App *app) {
  auto *create_cmd = app->add_subcommand("create", "Create database");
  create_cmd->set_help_flag("-h,--help", "Show help for create command");
  struct options {
    std::string path;
    double user_period;
  };
  auto opts = std::make_shared<options>();
  create_cmd->add_option("path", opts->path, "Path to database")
      ->default_val(DATABASE_PATH_DEFAULT);
  create_cmd
      ->add_option("user_period", opts->user_period,
                   "Max. age (seconds) of jobs when computing users score")
      ->default_val(USER_PERIOD_DEFAULT);
  create_cmd->callback([opts] {
    auto db_res = ps_db_database_create(opts->path, opts->user_period);
    if (!ps_is_ok(db_res)) {
      ps_exit_error(db_res);
    }
  });
}

void register_database_commands(CLI::App *app) {
  auto *db_cmd = app->add_subcommand("database", "Manage database");
  db_cmd->require_subcommand(true);
  db_cmd->set_help_flag("-h,--help", "Show help for database commands");
  register_create_command(db_cmd);
}
