// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "decay.hpp"
#include "formula.hpp"
#include "init.hpp"
#include "score.hpp"
#include "timewin.hpp"
#include <fmt/format.h>

int main(int argc, char *argv[]) {
  CLI::App app{"Interact with Seanergys' flux priority plugin and database to "
               "manage priorities"};

  app.set_help_flag("-h,--help", "Show help for priorisea commands");
  app.require_subcommand(true);

  register_init_command(&app);
  register_formula_commands(&app);
  register_decay_commands(&app);
  register_score_commands(&app);
  register_timewin_commands(&app);

  CLI11_PARSE(app, argc, argv);
  return 0;
}
