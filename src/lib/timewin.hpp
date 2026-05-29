// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <flux/core.h>
#include <jansson.h>
#include <string>
#include <vector>

struct timewin {
  std::string name;
  std::string cron;
  int duration;
  double priority;
};

////

std::string timewin_to_str(timewin timewin_obj);

struct validate_cron_res {
  bool ok;
  std::string message;
};
validate_cron_res validate_cron(const std::string &expr, std::string &error);

struct validate_timewin_res {
  bool ok;
  std::string message;
};
validate_timewin_res validate_timewin(timewin timewin_obj);

struct timewin_from_str_res {
  bool ok;
  std::string message;
  timewin timewin_obj;
};
timewin_from_str_res timewin_from_str(const std::string &str);

struct store_timewin_res {
  bool ok;
  std::string message;
};
store_timewin_res store_timewin(timewin timewin_obj);

struct get_timewin_res {
  bool ok;
  std::string message;
  std::string name;
  timewin timewin_obj;
};
struct get_timewins_res {
  bool ok;
  std::string message;
  std::vector<get_timewin_res> res;
};
get_timewins_res get_timewins(std::vector<std::string> names);

struct del_timewins_res {
  bool ok;
  std::string message;
};
del_timewins_res del_timewins(std::vector<std::string> names);

get_timewins_res list_timewins();
