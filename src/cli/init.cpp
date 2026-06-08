// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "init.hpp"
#include "lib/decay.hpp"
#include "lib/formula.hpp"
#include "lib/score.hpp"
#include "util.hpp"
#include <string>

void register_init_command(CLI::App *app) {
  auto *init_cmd = app->add_subcommand("init", "Initialize the database");
  init_cmd->set_help_flag("-h,--help", "Show help for score commands");
  init_cmd->callback([] {
    auto init_decay_res = ps_db_decay_init();
    if (!ps_is_ok(init_decay_res)) {
      return cli_error(init_decay_res);
    }

    auto init_formulas_res = ps_db_formulas_init();
    if (!ps_is_ok(init_formulas_res)) {
      return cli_error(init_formulas_res);
    }

    auto init_score_res = ps_db_score_init();
    if (!ps_is_ok(init_score_res)) {
      return cli_error(init_score_res);
    }
  });
}
