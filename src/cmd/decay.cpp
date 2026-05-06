#include "decay.hpp"
#include <bits/stdc++.h>
#include <iostream>
#include <jansson.h>
#include <string>
#include "util.hpp"

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
static const std::string HELP_MSG =
    "Try 'priorisea decay -h' for more information.";
static int run_help(int argc, char *argv[]) {
  std::cout << USAGE_MSG << std::endl;
  std::cout << "Manage the decay of priority scores over time." << std::endl;
  std::cout << "Example: priorisea decay get" << std::endl;
  std::cout << std::endl;
  std::cout << "COMMAND list and interpretation:" << std::endl;
  std::cout << "  get: get decay parameters" << std::endl;
  std::cout << "  set: set decay parameters" << std::endl;
  std::cout << "  reset: reset decay parameters to default" << std::endl;
  return 0;
}

static const std::string GET_USAGE_MSG =
    "Usage: priorisea decay get [-h] [OPTIONS]";
static const std::string GET_HELP_MSG =
    "Try 'priorisea decay get -h' for more information.";
static int run_get(int argc, char **argv) {
  bool json = false;
  if(argc > 0 && strcmp(argv[0], "-h") == 0) {
      std::cout << GET_USAGE_MSG << std::endl;
      std::cout << "Retrieve the decay parameters for priority scoring from "
                   "the database."
                << std::endl;
      std::cout << "Example: priorisea decay get" << std::endl;
      std::cout << std::endl;
      std::cout << "OPTIONS list and interpretation:" << std::endl;
      std::cout << "  --json: output in JSON format" << std::endl;
      return 0;
  }
  for (int i = 0; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--json") {
      json = true;
    } else {
      std::cout << "Error: 'priorisea decay get " << arg
                << "' is not a valid option" << std::endl;
      std::cout << GET_USAGE_MSG << std::endl;
      std::cout << GET_HELP_MSG << std::endl;
      return -1;
    }
  }
  // TODO: retrieve decay from database
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
              << std::setw(priority_width) << header_priority << '\n';

    std::cout << std::string(index_width, '-') << "-+-"
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
static const std::string SET_HELP_MSG =
    "Try 'priorisea decay set -h' for more information.";
static int run_set(int argc, char **argv) {
  if (argc != 1) {
    std::cout << SET_USAGE_MSG << std::endl;
    std::cout << SET_HELP_MSG << std::endl;
    return -1;
  }
  std::string arg = argv[0];
  if (arg == "-h") {
    std::cout << SET_USAGE_MSG << std::endl;
    std::cout << "Set the decay parameters for priority scores." << std::endl;
    std::cout << "The decay expression is a list of 'duration:priority' pairs "
                 "separated by commas."
              << std::endl;
    std::cout << "Example: priorisea decay set 86400:1,604800:0.5,1209600:0.25"
              << std::endl;
    return 0;
  }

  decay_list d;
  std::stringstream ss(arg);
  std::string t;
  while (getline(ss, t, ',')) {
    auto pos = t.find(':');
    if (pos == std::string::npos) {
      std::cout << "Error: 'priorisea decay set ...'" << std::endl;
      std::cout << "  priority is missing"
                << std::endl;
      std::cout << SET_USAGE_MSG << std::endl;
      std::cout << SET_HELP_MSG << std::endl;
      return -1;
    }
    auto duration_str = t.substr(0, pos);
    auto priority_str = t.substr(pos + 1);
    int duration = 0;
    if (!parse_int(duration_str, duration)) {
      std::cout << "Error: 'priorisea decay set ...'" << std::endl;
      std::cout << "  duration `" << duration_str << "` is not a valid integer"
                << std::endl;
      std::cout << SET_USAGE_MSG << std::endl;
      std::cout << SET_HELP_MSG << std::endl;
      return -1;
    }
    if (duration < 0) {
      std::cout << "Error: 'priorisea decay set ...'" << std::endl;
      std::cout << "  duration `" << duration_str
                << "` is not a positive integer" << std::endl;
      std::cout << SET_USAGE_MSG << std::endl;
      std::cout << SET_HELP_MSG << std::endl;
      return -1;
    }

    double priority = 0;
    if (!parse_double(priority_str, priority)) {
      std::cout << "Error: 'priorisea decay set ...'" << std::endl;
      std::cout << "  priority `" << priority_str << "` is not a valid double"
                << std::endl;
      std::cout << SET_USAGE_MSG << std::endl;
      std::cout << SET_HELP_MSG << std::endl;
      return -1;
    }
    d.push_back({.duration = duration, .priority = priority});
  }
  // TODO: store decay list into the database
  return 0;
}

static const std::string RESET_USAGE_MSG = "Usage: priorisea decay reset [-h]";
static const std::string RESET_HELP_MSG =
    "Try 'priorisea decay reset -h' for more information.";
static int run_reset(int argc, char **argv) {
  if (argc > 1) {
    std::cout << SET_USAGE_MSG << std::endl;
    std::cout << SET_HELP_MSG << std::endl;
    return -1;
  }
  std::string arg = argv[0];
  if (arg == "-h") {
    std::cout << RESET_USAGE_MSG << std::endl;
    std::cout << "Reset the decay parameters for priority scores." << std::endl;
    return 0;
  }

  // TODO: store DEFAULT_DECAY in the database
  return 0;
}

int run_decay(int argc, char **argv) {
  if (argc < 1) {
    std::cout << USAGE_MSG << std::endl;
    std::cout << HELP_MSG << std::endl;
    return -1;
  }

  std::string cmd = argv[0];
  if (cmd == "-h") {
    return run_help(argc - 1, argv + 1);
  } else if (cmd == "get") {
    return run_get(argc - 1, argv + 1);
  } else if (cmd == "set") {
    return run_set(argc - 1, argv + 1);
  } else if (cmd == "reset") {
    return run_reset(argc - 1, argv + 1);
  } else {
    std::cout << "Error: 'priorisea decay " << cmd << "' is not a valid command"
              << std::endl;
    std::cout << USAGE_MSG << std::endl;
    std::cout << HELP_MSG << std::endl;
    return -1;
  }
}
