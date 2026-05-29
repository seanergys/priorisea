// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "timewin.hpp"
#include "croncpp.h"
#include "flux.hpp"
#include "util.hpp"
#include <flux/core.h>
#include <iostream>
#include <jansson.h>
#include <sstream>
#include <string.h>
#include <string>
#include <vector>

std::string timewin_to_str(timewin timewin_obj) {
  std::stringstream ss;
  ss << timewin_obj.name << "," << timewin_obj.cron << ","
     << timewin_obj.duration << "," << timewin_obj.priority;
  return ss.str();
}

validate_cron_res validate_cron(const std::string &expr, std::string &error) {
  validate_cron_res res;
  try {
    cron::make_cron(expr);
    res.ok = true;
    return res;
  } catch (cron::bad_cronexpr const &ex) {
    res.ok = false;
    res.message = ex.what();
    return res;
  }
}

validate_timewin_res validate_timewin(timewin timewin_obj) {
  validate_timewin_res res;
  if (timewin_obj.name.empty()) {
    res.ok = false;
    res.message = "timewin_obj name is missing.";
    return res;
  }
  auto cron_res = validate_cron(timewin_obj.cron, res.message);
  if (!cron_res.ok) {
    res.ok = false;
    res.message = cron_res.message;
    return res;
  }
  if (timewin_obj.duration < 0) {
    res.ok = false;
    res.message = "duration is not a positive integer";
    return res;
  }
  res.ok = true;
  return res;
}

timewin_from_str_res timewin_from_str(const std::string &str) {
  timewin_from_str_res res;

  std::stringstream ss(str);
  std::string t;
  if (!getline(ss, t, ',')) {
    res.ok = false;
    res.message = "name is missing";
    return res;
  }
  res.timewin_obj.name = t;

  if (!getline(ss, t, ',')) {
    res.ok = false;
    res.message = "cron is missing";
    return res;
  }
  res.timewin_obj.cron = t;

  if (!getline(ss, t, ',')) {
    res.ok = false;
    res.message = "duration is missing";
    return res;
  }
  int duration = 0;
  if (!parse_int(t, duration)) {
    res.ok = false;
    res.message = "duration `" + t + "` is not a valid integer";
    return res;
  }
  res.timewin_obj.duration = duration;

  if (!getline(ss, t, ',')) {
    res.ok = false;
    res.message = "priority is missing";
    return res;
  }
  double priority = 0;
  if (!parse_double(t, priority)) {
    res.ok = false;
    res.message = "priority `" + t + "` is not a valid double";
    return res;
  }
  res.timewin_obj.priority = priority;

  auto validate_res = validate_timewin(res.timewin_obj);
  if (!validate_res.ok) {
    res.ok = false;
    res.message = validate_res.message;
    return res;
  }

  res.ok = true;
  return res;
}

store_timewin_res store_timewin(timewin timewin_obj) {
  store_timewin_res res;

  std::string timewin_str = timewin_to_str(timewin_obj);

  auto txn = flux_kvs_txn_create();
  if (!txn) {
    res.ok = false;
    res.message = "Failed to create KVS transaction.";
    return res;
  }
  if (flux_kvs_txn_put(txn, 0,
                       ("priorisea.timewin." + timewin_obj.name).c_str(),
                       timewin_str.c_str()) < 0) {
    res.ok = false;
    res.message = "Failed to store timewin in KVS transaction.";
    return res;
  }
  auto commit_res = kvs_commit(txn);
  if (commit_res != FLUX_KVS_ERR_SUCCESS) {
    res.ok = false;
    res.message = "Failed to commit timewin to KVS.";
    return res;
  }
  res.ok = true;
  return res;
}

get_timewins_res get_timewins(std::vector<std::string> names) {
  get_timewins_res res;
  res.ok = false;

  std::vector<std::string> keys;
  for (auto timewin_name : names) {
    keys.push_back("priorisea.timewin." + timewin_name);
  }

  auto lookup_res = kvs_lookup(keys);
  if (lookup_res.err != FLUX_KVS_ERR_SUCCESS) {
    res.message = "failed to lookup timewins in KVS. Unknown error";
    return res;
  }

  for (const auto &lookup_key_res : lookup_res.res) {
    get_timewin_res timewin_res;
    timewin_res.ok = false;

    size_t pos = lookup_key_res.key.rfind('.');
    std::string timewin_name = (pos == std::string::npos)
                                   ? lookup_key_res.key
                                   : lookup_key_res.key.substr(pos + 1);
    timewin_res.name = timewin_name;

    if (lookup_key_res.err != FLUX_KVS_ERR_SUCCESS) {
      timewin_res.message = "Failed to lookup timewin '" + timewin_name + "'";
      res.res.push_back(timewin_res);
      continue;
    }

    auto from_res = timewin_from_str(lookup_key_res.value);
    if (!from_res.ok) {
      timewin_res.message = from_res.message;
      res.res.push_back(timewin_res);
      continue;
    }

    timewin_res.ok = true;
    timewin_res.timewin_obj = from_res.timewin_obj;
    res.res.push_back(timewin_res);
  }

  res.ok = true;
  return res;
}

del_timewins_res del_timewins(std::vector<std::string> names) {
  del_timewins_res res;
  std::vector<std::string> keys;
  for (auto name : names) {
    keys.push_back("priorisea.timewin." + name);
  }
  auto remove_res = kvs_remove(keys);
  if (remove_res != FLUX_KVS_ERR_SUCCESS) {
    res.ok = false;
    res.message = "failed to delete times";
    return res;
  }
  res.ok = true;
  return res;
}

get_timewins_res list_timewins() {
  get_timewins_res res;
  res.ok = false;

  auto ls_res = flux_kvs_ls_dir("priorisea.timewin");
  if (!ls_res.ok) {
    res.message = ls_res.message;
    return res;
  }

  return get_timewins(ls_res.keys);
}
