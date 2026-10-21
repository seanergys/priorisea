// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/cli/commands/daemon.hpp"
#include "src/cli/commands/database.hpp"
#include "src/cli/commands/formula.hpp"
#include "src/cli/commands/job.hpp"
#include "src/cli/commands/plugin.hpp"
#include "src/cli/commands/timewin.hpp"
#include "src/cli/commands/user.hpp"
#include <CLI/CLI.hpp>

int main(int argc, char *argv[]) {
  CLI::App app{"Interact with Seanergys' flux priority plugin and database to "
               "manage priorities"};

  app.set_help_flag("-h,--help", "Show help for priorisea commands");
  app.require_subcommand(true);

  register_daemon_commands(&app);
  register_database_commands(&app);
  register_formula_commands(&app);
  register_job_commands(&app);
  register_plugin_commands(&app);
  register_timewin_commands(&app);
  register_user_commands(&app);

  CLI11_PARSE(app, argc, argv);
  return 0;
}
