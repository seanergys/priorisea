// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "croncpp.h"
#include "error.hpp"
#include "types.hpp"
#include <flux/core.h>
#include <jansson.h>
#include <string>
#include <vector>

struct ps_db_timewin_get_entry_t {
  std::string name;
  ps_result_t<ps_timewin_t> result;
};
using ps_db_timewins_get_t = std::vector<ps_db_timewin_get_entry_t>;

struct ps_db_timewin_del_entry_t {
  std::string name;
  ps_result_t<ps_success_t> result;
};
using ps_db_timewins_del_t = std::vector<ps_db_timewin_del_entry_t>;

////

std::string ps_timewin_to_str(const ps_timewin_t &timewin);

ps_result_t<ps_success_t> ps_validate_cron(const std::string &expr);

ps_result_t<ps_success_t> ps_validate_timewin(const ps_timewin_t &timewin);

ps_result_t<ps_timewin_t> ps_timewin_from_str(const std::string &str);

ps_result_t<ps_success_t> ps_db_timewin_set(const ps_timewin_t &timewin);

ps_result_t<ps_db_timewins_get_t>
ps_db_timewins_get(const std::vector<std::string> &names);

ps_result_t<ps_timewin_t> ps_db_timewin_get(const std::string &name);

ps_result_t<ps_db_timewins_del_t>
ps_db_timewins_del(const std::vector<std::string> &names);

ps_result_t<ps_success_t> ps_db_timewin_del(const std::string &name);

ps_result_t<ps_db_timewins_get_t> ps_db_timewins_list();

double ps_cron_prev(const cron::cronexpr &cron, double timestamp);
