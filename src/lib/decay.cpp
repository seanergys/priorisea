// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "decay.hpp"
#include "error.hpp"
#include "flux.hpp"
#include "util.hpp"
#include <flux/core.h>
#include <sstream>

static const ps_decay_t DEFAULT_DECAY = {
    {.duration = 86400, .priority = 1},
    {.duration = 604800, .priority = 0.5},
    {.duration = 1209600, .priority = 0.25},
};

////

std::string ps_decay_to_str(const ps_decay_t &decay) {
  std::stringstream ss;
  bool first = true;
  for (auto &d : decay) {
    if (!first) {
      ss << ",";
    } else {
      first = false;
    }
    ss << d.duration << ":" << d.priority;
  }
  return ss.str();
}

ps_result_t<ps_decay_t> ps_decay_from_str(const std::string &decay_str) {
  if (decay_str.empty()) {
    return ps_error(ps_str_empty_t{});
  }

  ps_decay_t res;
  std::stringstream ss(decay_str);
  std::string t;
  while (getline(ss, t, ',')) {
    auto pos = t.find(':');
    if (pos == std::string::npos) {
      return ps_error(ps_decay_priority_missing_t{});
    }

    auto duration_str = t.substr(0, pos);
    auto priority_str = t.substr(pos + 1);

    int duration = 0;
    if (!parse_int(duration_str, duration)) {
      return ps_error(ps_parse_int_err_t{.int_str = duration_str});
    }

    if (duration < 0) {
      return ps_error(ps_decay_duration_invalid_t{.value = duration});
    }

    double priority = 0;
    if (!parse_double(priority_str, priority)) {
      return ps_error(ps_parse_double_err_t{.double_str = priority_str});
    }

    res.push_back({.duration = duration, .priority = priority});
  }
  return res;
}

ps_result_t<ps_success_t> ps_db_decay_set(const ps_decay_t &decay) {
  std::string decay_str = ps_decay_to_str(decay);
  auto txn = flux_kvs_txn_create();
  if (!txn) {
    return ps_error(ps_flux_kvs_txn_create_err_t{.msg = strerror(errno)});
  }
  auto key = "priorisea.decay";
  auto value = decay_str.c_str();
  if (flux_kvs_txn_put(txn, 0, key, value) < 0) {
    return ps_error(ps_flux_kvs_txn_put_err_t{
        .key = key, .value = value, .msg = strerror(errno)});
  }
  return ps_flux_kvs_commit(txn);
}

ps_result_t<ps_decay_t> ps_db_decay_get() {
  auto lookup_res = ps_flux_kvs_lookup("priorisea.decay");
  if (!ps_is_ok(lookup_res)) {
    return ps_forward_error<ps_decay_t>(lookup_res,
                                        "getting stored decay value");
  }
  auto decay_str = ps_unwrap(lookup_res);

  auto parse_res = ps_decay_from_str(decay_str);
  if (!ps_is_ok(parse_res)) {
    return ps_forward_error<ps_decay_t>(parse_res,
                                        "parsing stored decay value");
  }
  return ps_unwrap(parse_res);
}

bool ps_db_is_decay() { return ps_is_ok(ps_db_decay_get()); }

ps_result_t<ps_success_t> ps_db_decay_reset() {
  return ps_db_decay_set(DEFAULT_DECAY);
}

ps_result_t<ps_success_t> ps_db_decay_init() {
  if (!ps_db_is_decay()) {
    return ps_db_decay_reset();
  }
  return ps_success_t{};
}
