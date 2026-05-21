// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "decay.hpp"
#include "lib/table.hpp"
#include "util.hpp"
#include <bits/stdc++.h>
#include <flux/core.h>
#include <iostream>
#include <jansson.h>
#include <lib/kvs.hpp>
#include <string>

struct decay_one {
  int duration;
  double priority;
};
typedef std::vector<decay_one> decay;
static const decay DEFAULT_DECAY = {
    {.duration = 86400, .priority = 1},
    {.duration = 604800, .priority = 0.5},
    {.duration = 1209600, .priority = 0.25},
};

////

static std::string decay_to_str(decay &decay) {
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

struct decay_from_str_res {
  bool ok;
  std::string message;
  decay decay_obj;
};
static decay_from_str_res decay_from_str(std::string decay_str) {
  decay_from_str_res res;
  std::stringstream ss(decay_str);
  std::string t;
  while (getline(ss, t, ',')) {
    auto pos = t.find(':');
    if (pos == std::string::npos) {
      res.ok = false;
      res.message = "priority is missing";
      return res;
    }

    auto duration_str = t.substr(0, pos);
    auto priority_str = t.substr(pos + 1);

    int duration = 0;
    if (!parse_int(duration_str, duration)) {
      res.ok = false;
      res.message = "duration `" + duration_str + "` is not a valid integer";
      return res;
    }

    if (duration < 0) {
      res.ok = false;
      res.message = "duration `" + duration_str + "` is not a positive integer";
      return res;
    }

    double priority = 0;
    if (!parse_double(priority_str, priority)) {
      res.ok = false;
      res.message = "priority `" + priority_str + "` is not a valid double";
      return res;
    }

    res.decay_obj.push_back({.duration = duration, .priority = priority});
  }
  return res;
}

struct store_decay_res {
  bool ok;
  std::string message;
};
static store_decay_res store_decay(decay decay) {
  store_decay_res res;

  std::string decay_str = decay_to_str(decay);

  auto txn = flux_kvs_txn_create();
  if (!txn) {
    res.ok = false;
    res.message = "Failed to create KVS transaction.";
    return res;
  }
  if (flux_kvs_txn_put(txn, 0, "priorisea.decay", decay_str.c_str()) < 0) {
    res.ok = false;
    res.message = "Failed to store decay parameters in KVS.";
    return res;
  }
  auto commit_res = kvs_commit(txn);
  if (commit_res != KVS_ERR_SUCCESS) {
    res.ok = false;
    res.message = "Failed to commit decay parameters to KVS.";
    return res;
  }
  res.ok = true;
  return res;
}

////

static const std::string USAGE_MSG = "Usage: priorisea decay [-h] COMMAND ARGS";
static const std::string TRY_HELP_MSG =
    "Try 'priorisea decay -h' for more information.";
static const std::string HELP_MSG = USAGE_MSG + R"(

Manage the decay of priority scores over time.
Example: priorisea decay get
COMMAND list and interpretation:
  get: get decay parameters
  set: set decay parameters
  reset: reset decay parameters to default)";

static const std::string GET_USAGE_MSG =
    "Usage: priorisea decay get [-h] [OPTIONS]";
static const std::string GET_TRY_HELP_MSG =
    "Try 'priorisea decay get -h' for more information.";
static const std::string GET_HELP_MSG = GET_USAGE_MSG + R"(

Retrieve the decay parameters for priority scoring from the database.
Example: priorisea decay get
OPTIONS list and interpretation:
  --json: output in JSON format)";

static int run_get(int argc, char **argv) {
  bool json = false;
  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << GET_HELP_MSG << std::endl;
    return 0;
  }
  for (int i = 0; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--json") {
      json = true;
    } else {
      std::cout << "Error: 'priorisea decay get " << arg
                << "' is not a valid option" << '\n'
                << GET_USAGE_MSG << '\n'
                << GET_TRY_HELP_MSG << std::endl;
      return -1;
    }
  }

  std::string decay_str;

  flux_t *h;
  if (!(h = flux_open(NULL, 0))) {
    return -1;
  }
  flux_future_t *f;
  if (!(f = flux_kvs_lookup(h, NULL, 0, "priorisea.decay"))) {
    flux_close(h);
    return -1;
  }
  const char *decay_str_tmp;
  if (flux_kvs_lookup_get(f, &decay_str_tmp) < 0) {
    flux_future_destroy(f);
    flux_close(h);
    return -1;
  }
  decay_str = decay_str_tmp;
  flux_future_destroy(f);
  flux_close(h);

  auto res = decay_from_str(decay_str);
  if (!res.ok) {
    std::cout << "Error: 'priorisea decay get ...'" << '\n'
              << "  " << res.message << '\n'
              << GET_USAGE_MSG << '\n'
              << GET_TRY_HELP_MSG << std::endl;
    return -1;
  }
  decay decay = res.decay_obj;

  if (json) {
    auto obj = json_array();
    for (const auto &d : decay) {
      auto dec_one = json_object();
      json_object_set_new(dec_one, "duration", json_integer(d.duration));
      json_object_set_new(dec_one, "priority", json_real(d.priority));
      json_array_append(obj, dec_one);
    }
    std::cout << json_dumps(obj, JSON_INDENT(2)) << std::endl;
    json_decref(obj);
  } else {
    print_table(decay, {{"Index",
                         [](auto &value, auto index) -> auto {
                           return std::to_string(index);
                         }},
                        {"Duration",
                         [](auto &value, auto index) -> auto {
                           return std::to_string(value.duration);
                         }},
                        {"Priority", [](auto &value, auto index) -> auto {
                           return std::to_string(value.priority);
                         }}});
  }

  return 0;
}

static const std::string SET_USAGE_MSG =
    "Usage: priorisea decay set [-h] DECAY";
static const std::string SET_TRY_HELP_MSG =
    "Try 'priorisea decay set -h' for more information.";
static const std::string SET_HELP_MSG = SET_USAGE_MSG + R"(
Set the decay parameters for priority scores.
The decay expression is a list of 'duration:priority' pairs separated by commas.
Example: priorisea decay set 86400:1,604800:0.5,1209600:0.25)";

static int run_set(int argc, char **argv) {
  if (argc != 1) {
    std::cout << SET_USAGE_MSG << '\n' << SET_TRY_HELP_MSG << std::endl;
    return -1;
  }

  std::string arg = argv[0];
  if (arg == "-h") {
    std::cout << SET_HELP_MSG << std::endl;
    return 0;
  }

  auto parse_res = decay_from_str(arg);
  if (!parse_res.ok) {
    std::cerr << "Error: 'priorisea decay set ...'" << '\n'
              << "  " << parse_res.message << '\n'
              << SET_USAGE_MSG << '\n'
              << SET_TRY_HELP_MSG << std::endl;
    return -1;
  }
  decay decay = parse_res.decay_obj;

  auto store_res = store_decay(decay);
  if (!store_res.ok) {
    std::cerr << "Error: 'priorisea decay set ...'" << '\n'
              << "  " << store_res.message << '\n'
              << SET_USAGE_MSG << '\n'
              << SET_TRY_HELP_MSG << std::endl;
    return -1;
  }

  return 0;
}

static const std::string RESET_USAGE_MSG = "Usage: priorisea decay reset [-h]";
static const std::string RESET_TRY_HELP_MSG =
    "Try 'priorisea decay reset -h' for more information.";
static const std::string RESET_HELP_MSG = RESET_USAGE_MSG + R"(
Reset the decay parameters for priority scores.)";

static int run_reset(int argc, char **argv) {
  if (argc > 1) {
    std::cout << RESET_USAGE_MSG << '\n' << RESET_TRY_HELP_MSG << std::endl;
    return -1;
  }

  if (argc > 0 && std::string(argv[0]) == "-h") {
    std::cout << RESET_HELP_MSG << std::endl;
    return 0;
  }

  auto store_res = store_decay(DEFAULT_DECAY);
  if (!store_res.ok) {
    std::cerr << "Error: 'priorisea decay reset ...'" << '\n'
              << "  " << store_res.message << '\n'
              << RESET_USAGE_MSG << '\n'
              << RESET_TRY_HELP_MSG << std::endl;
    return -1;
  }

  return 0;
}

int run_decay(int argc, char **argv) {
  if (argc < 1) {
    std::cout << USAGE_MSG << '\n' << TRY_HELP_MSG << std::endl;
    return -1;
  }

  std::string cmd = argv[0];
  if (cmd == "-h") {
    std::cout << HELP_MSG << std::endl;
    return 0;
  } else if (cmd == "get") {
    return run_get(argc - 1, argv + 1);
  } else if (cmd == "set") {
    return run_set(argc - 1, argv + 1);
  } else if (cmd == "reset") {
    return run_reset(argc - 1, argv + 1);
  } else {
    std::cout << "Error: 'priorisea decay " << cmd << "' is not a valid command"
              << '\n'
              << USAGE_MSG << '\n'
              << TRY_HELP_MSG << std::endl;
    return -1;
  }
}
