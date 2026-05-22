// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

struct init_decay_res {
  bool ok;
  std::string message;
};

int run_decay(int argc, char *argv[]);
init_decay_res init_decay();
