#include "decay.hpp"
#include "util.hpp"
#include <bits/stdc++.h>
#include <iostream>
#include <jansson.h>
#include <string>

struct decay_one {
  int duration;
  double priority;
};
typedef std::vector<decay_one> decay_list;
static const decay_list DEFAULT_DECAY = {
    {.duration = 86400, .priority = 1},
    {.duration = 604800, .priority = 0.5},
    {.duration = 1209600, .priority = 0.25},
};

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

  // TODO: retrieve decay from database
  // TODO: to replace this fake list
  decay_list decay = {
      {.duration = 86400, .priority = 1},
      {.duration = 604800, .priority = 0.5},
      {.duration = 1209600, .priority = 0.25},
  };

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
    std::string header_index = "Index";
    std::string header_duration = "Duration";
    std::string header_priority = "Priority";

    std::size_t index_width = header_index.size();
    std::size_t duration_width = header_duration.size();
    std::size_t priority_width = header_priority.size();

    index_width = std::max(index_width, std::to_string(decay.size()).size());
    for (const auto &d : decay) {
      duration_width =
          std::max(duration_width, std::to_string(d.duration).size());
      priority_width =
          std::max(priority_width, std::to_string(d.priority).size());
    }

    std::cout << std::left << std::setw(index_width) << header_index << " | "
              << std::setw(duration_width) << header_duration << " | "
              << std::setw(priority_width) << header_priority << '\n'
              << std::string(index_width, '-') << "-+-"
              << std::string(duration_width, '-') << "-+-"
              << std::string(priority_width, '-') << '\n';

    for (int i = 0; i < decay.size(); i++) {
      std::cout << std::left << std::setw(index_width) << i << " | "
                << std::setw(duration_width) << decay[i].duration << " | "
                << std::setw(priority_width) << decay[i].priority << '\n';
    }

    std::cout << std::endl;
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

  decay_list d;
  std::stringstream ss(arg);
  std::string t;
  while (getline(ss, t, ',')) {
    auto pos = t.find(':');
    if (pos == std::string::npos) {
      std::cout << "Error: 'priorisea decay set ...'" << '\n'
                << "  priority is missing" << '\n'
                << SET_USAGE_MSG << '\n'
                << SET_TRY_HELP_MSG << std::endl;
      return -1;
    }

    auto duration_str = t.substr(0, pos);
    auto priority_str = t.substr(pos + 1);

    int duration = 0;
    if (!parse_int(duration_str, duration)) {
      std::cout << "Error: 'priorisea decay set ...'" << '\n'
                << "  duration `" << duration_str << "` is not a valid integer"
                << '\n'
                << SET_USAGE_MSG << '\n'
                << SET_TRY_HELP_MSG << std::endl;
      return -1;
    }

    if (duration < 0) {
      std::cout << "Error: 'priorisea decay set ...'" << '\n'
                << "  duration `" << duration_str
                << "` is not a positive integer" << '\n'
                << SET_USAGE_MSG << '\n'
                << SET_TRY_HELP_MSG << std::endl;
      return -1;
    }

    double priority = 0;
    if (!parse_double(priority_str, priority)) {
      std::cout << "Error: 'priorisea decay set ...'" << '\n'
                << "  priority `" << priority_str << "` is not a valid double"
                << '\n'
                << SET_USAGE_MSG << '\n'
                << SET_TRY_HELP_MSG << std::endl;
      return -1;
    }

    d.push_back({.duration = duration, .priority = priority});
  }

  // TODO: store decay list into the database
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

  // TODO: store DEFAULT_DECAY in the database
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
