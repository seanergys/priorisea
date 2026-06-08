// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "timewin.hpp"
#include "croncpp.h"
#include "error.hpp"
#include "flux.hpp"
#include "util.hpp"
#include <cerrno>
#include <flux/core.h>
#include <iostream>
#include <jansson.h>
#include <sstream>
#include <string.h>
#include <string>
#include <variant>
#include <vector>

std::string ps_timewin_to_str(const ps_timewin_t &timewin) {
  std::stringstream ss;
  ss << timewin.name << "," << timewin.cron << "," << timewin.duration << ","
     << timewin.priority;
  return ss.str();
}

ps_result_t<ps_success_t> ps_validate_cron(const std::string &expr) {
  try {
    cron::make_cron(expr);
    return ps_success_t{};
  } catch (cron::bad_cronexpr const &ex) {
    return ps_error(ps_cron_parse_err_t{.cron = ex.what()});
  }
}

ps_result_t<ps_success_t> ps_validate_timewin(const ps_timewin_t &timewin) {
  if (timewin.name.empty()) {
    return ps_error(ps_str_empty_t{});
  }
  auto cron_res = ps_validate_cron(timewin.cron);
  if (!ps_is_ok(cron_res)) {
    return cron_res;
  }
  if (timewin.duration < 0) {
    return ps_error(ps_timewin_neg_duration_t{.duration = timewin.duration});
  }
  return ps_success_t{};
}

ps_result_t<ps_timewin_t> ps_timewin_from_str(const std::string &str) {
  ps_timewin_t timewin;
  std::stringstream ss(str);
  std::string t;
  if (!getline(ss, t, ',')) {
    return ps_error(ps_timewin_name_missing_t{});
  }
  timewin.name = t;

  if (!getline(ss, t, ',')) {
    return ps_error(ps_timewin_cron_missing_t{});
  }
  timewin.cron = t;

  if (!getline(ss, t, ',')) {
    return ps_error(ps_timewin_duration_missing_t{});
  }
  int duration = 0;
  if (!parse_int(t, duration)) {
    return ps_error(ps_parse_int_err_t{});
  }
  timewin.duration = duration;

  if (!getline(ss, t, ',')) {
    return ps_error(ps_timewin_priority_missing_t{});
  }
  double priority = 0;
  if (!parse_double(t, priority)) {
    return ps_error(ps_timewin_priority_parse_err_t{});
  }
  timewin.priority = priority;

  auto validate_res = ps_validate_timewin(timewin);
  if (!ps_is_ok(validate_res)) {
    return ps_forward_error<ps_timewin_t>(validate_res, "validating timewin");
  }
  return timewin;
}

ps_result_t<ps_success_t> ps_db_timewin_set(const ps_timewin_t &timewin) {
  auto timewin_str = ps_timewin_to_str(timewin);

  flux_kvs_txn_t *txn;
  if (!(txn = flux_kvs_txn_create())) {
    return ps_error(ps_flux_txn_create_err_t{.msg = strerror(errno)});
  }
  auto key = "priorisea.timewin." + timewin.name;
  auto value = timewin_str;
  if (flux_kvs_txn_put(txn, 0, key.c_str(), value.c_str()) < 0) {
    return ps_error(ps_flux_kvs_txn_put_err_t{
        .key = key, .value = value, .msg = strerror(errno)});
  }
  return ps_flux_kvs_commit(txn);
}

ps_result_t<ps_db_timewins_get_t>
ps_db_timewins_get(const std::vector<std::string> &names) {
  std::vector<std::string> keys;
  for (auto timewin_name : names) {
    keys.push_back("priorisea.timewin." + timewin_name);
  }

  auto lookup_res = ps_flux_kvs_lookup(keys);
  if (!ps_is_ok(lookup_res)) {
    std::stringstream ss;
    ss << "looking up timewin keys in flux kvs: ";
    for (const auto timewin_name : names) {
      ss << timewin_name << " ";
    }
    return ps_forward_error<ps_db_timewins_get_t>(lookup_res, ss.str());
  }

  ps_db_timewins_get_t res;
  for (const auto &lookup_key_res : ps_unwrap(lookup_res)) {
    size_t pos = lookup_key_res.key.rfind('.');
    std::string timewin_name = (pos == std::string::npos)
                                   ? lookup_key_res.key
                                   : lookup_key_res.key.substr(pos + 1);

    if (!ps_is_ok(lookup_key_res.value)) {
      res.push_back(
          {.name = timewin_name,
           .result = ps_forward_error<ps_timewin_t>(
               lookup_key_res.value,
               "getting timewin '" + timewin_name + "' from flux kvs")});
      continue;
    }

    auto lkv = lookup_key_res.value;
    auto lookup_key_value = ps_unwrap(lkv);

    auto from_res = ps_timewin_from_str(lookup_key_value);
    res.push_back({.name = timewin_name, .result = from_res});
  }

  return res;
}

ps_result_t<ps_timewin_t> ps_db_timewin_get(const std::string &name) {
  auto get_res = ps_db_timewins_get({name});
  if (!ps_is_ok(get_res)) {
    return ps_forward_error<ps_timewin_t>(get_res, "getting timewin '" + name +
                                                       "' from database");
  }
  return ps_unwrap(get_res)[0].result;
}

ps_result_t<ps_db_timewins_del_t>
ps_db_timewins_del(const std::vector<std::string> &names) {
  std::vector<std::string> keys;
  for (auto name : names) {
    keys.push_back("priorisea.timewin." + name);
  }
  auto remove_res = ps_flux_kvs_remove(keys);
  if (!ps_is_ok(remove_res)) {
    std::stringstream ss;
    ss << "removing timewin keys from flux kvs: ";
    for (const auto timewin_name : names) {
      ss << timewin_name << " ";
    }
    return ps_forward_error<ps_db_timewins_del_t>(remove_res, ss.str());
  }
  ps_db_timewins_del_t res;
  for (const auto &kv : ps_unwrap(remove_res)) {
    size_t pos = kv.key.rfind('.');
    std::string timewin_name =
        (pos == std::string::npos) ? kv.key : kv.key.substr(pos + 1);
    res.push_back({.name = timewin_name, .result = kv.result});
  }
  return res;
}

ps_result_t<ps_success_t> ps_db_timewin_del(const std::string &name) {
  auto del_res = ps_db_timewins_del({name});
  if (!ps_is_ok(del_res)) {
    return ps_forward_error<ps_success_t>(del_res, "deleting timewin '" + name +
                                                       "' from database");
  }
  return ps_success_t{};
}

ps_result_t<ps_db_timewins_get_t> ps_db_timewins_list() {
  auto ls_res = ps_flux_kvs_ls_dir("priorisea.timewin");
  if (!ps_is_ok(ls_res)) {
    return ps_forward_error<ps_db_timewins_get_t>(
        ls_res, "listing timewins from flux kvs");
  }
  return ps_db_timewins_get(ps_unwrap(ls_res));
}
