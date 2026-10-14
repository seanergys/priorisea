// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/cli/commands/database.hpp"
#include "src/database/decay.hpp"
#include "src/database/formula.hpp"
#include "src/database/info.hpp"
#include "src/database/sqlite/sqlite.hpp"
#include "src/lib/error.hpp"
#include <memory>
#include <string>

static void register_create_command(CLI::App *app) {
  auto *create_cmd = app->add_subcommand("create", "Create database");
  create_cmd->set_help_flag("-h,--help", "Show help for create command");
  struct options {
    std::string path;
  };
  auto opts = std::make_shared<options>();
  create_cmd->add_option("path", opts->path, "Path to database")
      ->default_val(DATABASE_PATH_DEFAULT);
  create_cmd->callback([opts] {
    auto db_res = ps_db_sqlite_create(opts->path);
    if (!ps_is_ok(db_res)) {
      ps_exit_error(db_res);
    }
    auto db = ps_unwrap(db_res);
    auto decay_res = ps_db_decay_init(db);
    if (!ps_is_ok(decay_res)) {
      ps_exit_error(decay_res);
    }
    auto formulas_res = ps_db_formulas_init(db);
    if (!ps_is_ok(formulas_res)) {
      ps_exit_error(formulas_res);
    }
    auto info_res = ps_db_info_init(db);
    if (!ps_is_ok(info_res)) {
      ps_exit_error(info_res);
    }
  });
}

void register_database_commands(CLI::App *app) {
  auto *db_cmd = app->add_subcommand("database", "Manage database");
  db_cmd->require_subcommand(true);
  db_cmd->set_help_flag("-h,--help", "Show help for database commands");
  register_create_command(db_cmd);
}
