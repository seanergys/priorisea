// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <string>
#include <vector>

struct decay_one {
  int duration;
  double priority;
};

typedef std::vector<decay_one> decay;

const decay DEFAULT_DECAY = {
    {.duration = 86400, .priority = 1},
    {.duration = 604800, .priority = 0.5},
    {.duration = 1209600, .priority = 0.25},
};

////

std::string decay_to_str(decay &decay);

struct decay_from_str_res {
  bool ok;
  std::string message;
  decay decay_obj;
};
decay_from_str_res decay_from_str(std::string decay_str);

struct store_decay_res {
  bool ok;
  std::string message;
};
store_decay_res store_decay(decay decay);

struct get_decay_res {
  bool ok;
  std::string message;
  decay decay_obj;
};
get_decay_res get_decay();

bool is_decay_stored();

struct reset_decay_res {
  bool ok;
  std::string message;
};
reset_decay_res reset_decay();

struct init_decay_res {
  bool ok;
  std::string message;
};
init_decay_res init_decay();
