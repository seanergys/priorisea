// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "decay.hpp"
#include "flux.hpp"
#include "util.hpp"
#include <flux/core.h>
#include <sstream>

std::string decay_to_str(decay &decay) {
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

decay_from_str_res decay_from_str(std::string decay_str) {
  decay_from_str_res res;
  res.ok = false;

  if (decay_str.empty()) {
    res.message = "decay string is empty";
    return res;
  }

  std::stringstream ss(decay_str);
  std::string t;
  while (getline(ss, t, ',')) {
    auto pos = t.find(':');
    if (pos == std::string::npos) {
      res.message = "priority is missing";
      return res;
    }

    auto duration_str = t.substr(0, pos);
    auto priority_str = t.substr(pos + 1);

    int duration = 0;
    if (!parse_int(duration_str, duration)) {
      res.message = "duration `" + duration_str + "` is not a valid integer";
      return res;
    }

    if (duration < 0) {
      res.message = "duration `" + duration_str + "` is not a positive integer";
      return res;
    }

    double priority = 0;
    if (!parse_double(priority_str, priority)) {
      res.message = "priority `" + priority_str + "` is not a valid double";
      return res;
    }

    res.decay_obj.push_back({.duration = duration, .priority = priority});
  }
  res.ok = true;
  return res;
}

store_decay_res store_decay(decay decay) {
  store_decay_res res;
  res.ok = false;

  std::string decay_str = decay_to_str(decay);

  auto txn = flux_kvs_txn_create();
  if (!txn) {
    res.message = "Failed to create KVS transaction.";
    return res;
  }
  if (flux_kvs_txn_put(txn, 0, "priorisea.decay", decay_str.c_str()) < 0) {
    res.message = "Failed to store decay parameters in KVS.";
    return res;
  }
  auto commit_res = kvs_commit(txn);
  if (commit_res != FLUX_KVS_ERR_SUCCESS) {
    res.message = "Failed to commit decay parameters to KVS.";
    return res;
  }
  res.ok = true;
  return res;
}

get_decay_res get_decay() {
  get_decay_res res;
  res.ok = false;

  auto lookup_res = kvs_lookup({"priorisea.decay"});
  if (lookup_res.err != FLUX_KVS_ERR_SUCCESS) {
    res.message = "failed to lookup decay string in KVS. Unknown error";
    return res;
  }
  auto lookup_key_res = lookup_res.res[0];
  if (lookup_key_res.err != FLUX_KVS_ERR_SUCCESS) {
    res.message = "failed to lookup decay string in KVS. Unknown error";
    return res;
  }
  auto decay_str = lookup_key_res.value;

  auto parse_res = decay_from_str(decay_str);
  if (!parse_res.ok) {
    res.message = parse_res.message;
    return res;
  }
  res.ok = true;
  res.decay_obj = parse_res.decay_obj;
  return res;
}

bool is_decay_stored() { return get_decay().ok; }

reset_decay_res reset_decay() {
  reset_decay_res res;
  res.ok = false;
  auto store_res = store_decay(DEFAULT_DECAY);
  if (!store_res.ok) {
    res.message = store_res.message;
    return res;
  }
  res.ok = true;
  return res;
}

init_decay_res init_decay() {
  init_decay_res res;
  res.ok = false;
  if (is_decay_stored()) {
    res.ok = true;
    return res;
  }

  auto reset_res = reset_decay();
  if (!reset_res.ok) {
    res.message = reset_res.message;
    return res;
  }
  res.ok = true;
  return res;
}
