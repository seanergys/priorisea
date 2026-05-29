// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "decay.hpp"
#include "lib/decay.hpp"
#include "lib/util.hpp"
#include <bits/stdc++.h>
#include <iostream>
#include <jansson.h>
#include <string>

////

static const std::string INIT_USAGE_MSG = "Usage: priorisea decay init [-h]";
static const std::string INIT_TRY_HELP_MSG =
    "Try 'priorisea decay init -h' for more information.";
static const std::string INIT_HELP_MSG = INIT_USAGE_MSG + R"(

Initialize the decay parameters for priority scoring.
If the decay parameters are already stored, this command will not do anything.)";

static int run_init(int argc, char **argv) {
  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << INIT_HELP_MSG << std::endl;
    return 0;
  }
  if (argc > 0) {
    std::cerr << INIT_USAGE_MSG << '\n' << INIT_TRY_HELP_MSG << std::endl;
    return -1;
  }

  auto init_res = init_decay();
  if (!init_res.ok) {
    std::cerr << "Error: 'priorisea decay init" << '\n'
              << "  " << init_res.message << '\n'
              << INIT_USAGE_MSG << '\n'
              << INIT_TRY_HELP_MSG << std::endl;
    return -1;
  }

  return 0;
}

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
      std::cerr << "Error: 'priorisea decay get " << arg
                << "' is not a valid option" << '\n'
                << GET_USAGE_MSG << '\n'
                << GET_TRY_HELP_MSG << std::endl;
      return -1;
    }
  }

  auto get_res = get_decay();
  if (!get_res.ok) {
    std::cerr << "Error: 'priorisea decay get ...'" << '\n'
              << "  " << get_res.message << '\n'
              << GET_USAGE_MSG << '\n'
              << GET_TRY_HELP_MSG << std::endl;
    return -1;
  }
  decay decay = get_res.decay_obj;

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
This command will not update the users score values.
The decay expression is a list of 'duration:priority' pairs separated by commas.
Example: priorisea decay set 86400:1,604800:0.5,1209600:0.25)";

static int run_set(int argc, char **argv) {
  if (argc != 1) {
    std::cerr << SET_USAGE_MSG << '\n' << SET_TRY_HELP_MSG << std::endl;
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
  if (argc > 0 && std::string(argv[0]) == "-h") {
    std::cout << RESET_HELP_MSG << std::endl;
    return 0;
  }

  if (argc > 0) {
    std::cerr << RESET_USAGE_MSG << '\n' << RESET_TRY_HELP_MSG << std::endl;
    return -1;
  }

  auto reset_res = reset_decay();
  if (!reset_res.ok) {
    std::cerr << "Error: 'priorisea decay reset ...'" << '\n'
              << "  " << reset_res.message << '\n'
              << RESET_USAGE_MSG << '\n'
              << RESET_TRY_HELP_MSG << std::endl;
    return -1;
  }

  return 0;
}

static const std::string USAGE_MSG = "Usage: priorisea decay [-h] COMMAND ARGS";
static const std::string TRY_HELP_MSG =
    "Try 'priorisea decay -h' for more information.";
static const std::string HELP_MSG = USAGE_MSG + R"(

Manage the decay of priority scores over time.
Example: priorisea decay get
COMMAND list and interpretation:
  init: initialize decay parameters
  get: get decay parameters
  set: set decay parameters
  reset: reset decay parameters to default)";
int run_decay(int argc, char **argv) {
  if (argc < 1) {
    std::cerr << USAGE_MSG << '\n' << TRY_HELP_MSG << std::endl;
    return -1;
  }

  std::string cmd = argv[0];
  if (cmd == "-h") {
    std::cout << HELP_MSG << std::endl;
    return 0;
  } else if (cmd == "init") {
    return run_init(argc - 1, argv + 1);
  } else if (cmd == "get") {
    return run_get(argc - 1, argv + 1);
  } else if (cmd == "set") {
    return run_set(argc - 1, argv + 1);
  } else if (cmd == "reset") {
    return run_reset(argc - 1, argv + 1);
  } else {
    std::cerr << "Error: 'priorisea decay " << cmd << "' is not a valid command"
              << '\n'
              << USAGE_MSG << '\n'
              << TRY_HELP_MSG << std::endl;
    return -1;
  }
}
