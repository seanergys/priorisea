// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "timewin.hpp"
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

json_t *ps_timewin_to_json(const ps_timewin_t &timewin) {
  auto *obj = json_object();
  json_object_set_new(obj, "name", json_string(timewin.name.c_str()));
  json_object_set_new(obj, "cron", json_string(timewin.cron.c_str()));
  json_object_set_new(obj, "duration", json_integer(timewin.duration));
  json_object_set_new(obj, "score", json_real(timewin.score));
  return obj;
}

ps_timewin_t ps_timewin_from_json(json_t *obj) {
  ps_timewin_t res;
  const char *name = nullptr;
  const char *cron = nullptr;
  int duration = 0;
  double score = 0;
  json_unpack(obj, "{s:s, s:s, s:i, s:f}", "name", &name, "cron", &cron,
              "duration", &duration, "score", &score);
  res.name = name ? name : "";
  res.cron = cron ? cron : "";
  res.duration = duration;
  res.score = score;
  return res;
}

ps_result_t<ps_success_t> ps_db_timewin_set(const ps_timewin_t &timewin) {
  auto *json_timewin = ps_timewin_to_json(timewin);
  char *timewin_str = json_dumps(json_timewin, 0);
  json_decref(json_timewin);
  if (!timewin_str) {
    return ps_error(ps_json_pack_err_t{});
  }

  flux_kvs_txn_t *txn;
  if (!(txn = flux_kvs_txn_create())) {
    free(timewin_str);
    return ps_error(ps_flux_txn_create_err_t{.msg = strerror(errno)});
  }
  auto key = "priorisea.timewin." + timewin.name;
  if (flux_kvs_txn_put(txn, 0, key.c_str(), timewin_str) < 0) {
    free(timewin_str);
    return ps_error(ps_flux_kvs_txn_put_err_t{
        .key = key, .value = timewin_str, .msg = strerror(errno)});
  }
  free(timewin_str);
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

    json_error_t json_err;
    auto *json_timewin = json_loads(lookup_key_value.c_str(), 0, &json_err);
    if (!json_timewin) {
      res.push_back({.name = timewin_name,
                     .result = ps_error(
                         ps_json_unpack_err_t{.dumps = lookup_key_value})});
      continue;
    }

    auto timewin = ps_timewin_from_json(json_timewin);
    json_decref(json_timewin);
    res.push_back({.name = timewin_name, .result = timewin});
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

double ps_cron_prev(const cron::cronexpr &cron, double timestamp) {
  int step = 1;
  double low = timestamp - step;
  while (cron::cron_next(cron, low) >= timestamp) {
    step = step * 2;
    low = timestamp - step;
    if (low < 0) {
      return -1;
    }
  }
  double high = timestamp;
  while (low + 1 < high) {
    double mid = (low + high) / 2;
    if (cron::cron_next(cron, mid) < timestamp) {
      low = mid;
    } else {
      high = mid;
    }
  }
  return cron::cron_next(cron, low);
}
