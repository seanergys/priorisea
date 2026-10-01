// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "config.hpp"
#include "decay.hpp"
#include "formula.hpp"
#include "init.hpp"
#include "lib/config.hpp"
#include "plugin.hpp"
#include "score.hpp"
#include "timewin.hpp"

int main(int argc, char *argv[]) {
  CLI::App app{"Interact with Seanergys' flux priority plugin and database to "
               "manage priorities"};

  app.set_help_flag("-h,--help", "Show help for priorisea commands");
  app.require_subcommand(true);

  std::string config_path = ps_config_default_path();
  app.add_option("--config", config_path, "Path to config file");

  register_init_command(&app);
  register_config_commands(&app, config_path);
  register_formula_commands(&app);
  register_decay_commands(&app);
  register_score_commands(&app, config_path);
  register_timewin_commands(&app);
  register_plugin_commands(&app);

  CLI11_PARSE(app, argc, argv);
  return 0;
}
