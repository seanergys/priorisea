// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "time.hpp"
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

std::string timeframe_to_str(timeframe time) {
  std::stringstream ss;
  ss << time.name << "," << time.cron << "," << time.duration << ","
     << time.priority;
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

validate_timeframe_res validate_timeframe(timeframe time) {
  validate_timeframe_res res;
  if (time.name.empty()) {
    res.ok = false;
    res.message = "time name is missing.";
    return res;
  }
  auto cron_res = validate_cron(time.cron, res.message);
  if (!cron_res.ok) {
    res.ok = false;
    res.message = cron_res.message;
    return res;
  }
  if (time.duration < 0) {
    res.ok = false;
    res.message = "duration is not a positive integer";
    return res;
  }
  res.ok = true;
  return res;
}

timeframe_from_str_res timeframe_from_str(const std::string &str) {
  timeframe_from_str_res res;

  std::stringstream ss(str);
  std::string t;
  if (!getline(ss, t, ',')) {
    res.ok = false;
    res.message = "name is missing";
    return res;
  }
  res.timeframe_obj.name = t;

  if (!getline(ss, t, ',')) {
    res.ok = false;
    res.message = "cron is missing";
    return res;
  }
  res.timeframe_obj.cron = t;

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
  res.timeframe_obj.duration = duration;

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
  res.timeframe_obj.priority = priority;

  auto validate_res = validate_timeframe(res.timeframe_obj);
  if (!validate_res.ok) {
    res.ok = false;
    res.message = validate_res.message;
    return res;
  }

  res.ok = true;
  return res;
}

store_timeframe_res store_timeframe(timeframe time) {
  store_timeframe_res res;

  std::string timeframe_str = timeframe_to_str(time);

  auto txn = flux_kvs_txn_create();
  if (!txn) {
    res.ok = false;
    res.message = "Failed to create KVS transaction.";
    return res;
  }
  if (flux_kvs_txn_put(txn, 0, ("priorisea.timeframe." + time.name).c_str(),
                       timeframe_str.c_str()) < 0) {
    res.ok = false;
    res.message = "Failed to store time in KVS transaction.";
    return res;
  }
  auto commit_res = kvs_commit(txn);
  if (commit_res != FLUX_KVS_ERR_SUCCESS) {
    res.ok = false;
    res.message = "Failed to commit time to KVS.";
    return res;
  }
  res.ok = true;
  return res;
}

get_timeframes_res get_timeframes(std::vector<std::string> names) {
  get_timeframes_res res;
  res.ok = false;

  std::vector<std::string> keys;
  for (auto timeframe_name : names) {
    keys.push_back("priorisea.timeframe." + timeframe_name);
  }

  auto lookup_res = kvs_lookup(keys);
  if (lookup_res.err != FLUX_KVS_ERR_SUCCESS) {
    res.message = "failed to lookup timeframes in KVS. Unknown error";
    return res;
  }

  for (const auto &lookup_key_res : lookup_res.res) {
    get_timeframe_res timeframe_res;
    timeframe_res.ok = false;

    size_t pos = lookup_key_res.key.rfind('.');
    std::string timeframe_name = (pos == std::string::npos)
                                     ? lookup_key_res.key
                                     : lookup_key_res.key.substr(pos + 1);
    timeframe_res.name = timeframe_name;

    if (lookup_key_res.err != FLUX_KVS_ERR_SUCCESS) {
      timeframe_res.message =
          "Failed to lookup timeframe '" + timeframe_name + "'";
      res.res.push_back(timeframe_res);
      continue;
    }

    auto from_res = timeframe_from_str(lookup_key_res.value);
    if (!from_res.ok) {
      timeframe_res.message = from_res.message;
      res.res.push_back(timeframe_res);
      continue;
    }

    timeframe_res.ok = true;
    timeframe_res.timeframe_obj = from_res.timeframe_obj;
    res.res.push_back(timeframe_res);
  }

  res.ok = true;
  return res;
}

del_timeframes_res del_timeframes(std::vector<std::string> names) {
  del_timeframes_res res;
  std::vector<std::string> keys;
  for (auto name : names) {
    keys.push_back("priorisea.timeframe." + name);
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

get_timeframes_res list_timeframes() {
  get_timeframes_res res;
  res.ok = false;

  auto ls_res = flux_kvs_ls_dir("priorisea.timeframe");
  if (!ls_res.ok) {
    res.message = ls_res.message;
    return res;
  }

  return get_timeframes(ls_res.keys);
}
