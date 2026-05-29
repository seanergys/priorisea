// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <flux/core.h>
#include <jansson.h>
#include <string>
#include <vector>

struct timeframe {
  std::string name;
  std::string cron;
  int duration;
  double priority;
};

////

std::string timeframe_to_str(timeframe time);

struct validate_cron_res {
  bool ok;
  std::string message;
};
validate_cron_res validate_cron(const std::string &expr, std::string &error);

struct validate_timeframe_res {
  bool ok;
  std::string message;
};
validate_timeframe_res validate_timeframe(timeframe time);

struct timeframe_from_str_res {
  bool ok;
  std::string message;
  timeframe timeframe_obj;
};
timeframe_from_str_res timeframe_from_str(const std::string &str);

struct store_timeframe_res {
  bool ok;
  std::string message;
};
store_timeframe_res store_timeframe(timeframe time);

struct get_timeframe_res {
  bool ok;
  std::string message;
  std::string name;
  timeframe timeframe_obj;
};
struct get_timeframes_res {
  bool ok;
  std::string message;
  std::vector<get_timeframe_res> res;
};
get_timeframes_res get_timeframes(std::vector<std::string> names);

struct del_timeframes_res {
  bool ok;
  std::string message;
};
del_timeframes_res del_timeframes(std::vector<std::string> names);
