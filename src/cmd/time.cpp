#include "time.hpp"
#include "util.hpp"
#include <flux/core.h>
#include <iomanip>
#include <iostream>
#include <jansson.h>
#include <string>
#include <string.h>
#include <vector>

struct time {
  std::string name;
  std::string cron;
  int duration;
  double priority;
};

static const std::string USAGE_MSG =
    "Usage: priorisea time [-h] COMMAND ARGS";
static const std::string HELP_MSG =
    "Try 'priorisea time -h' for more information.";

static int run_help(int argc, char *argv[]) {
  std::cout << USAGE_MSG << std::endl;
  std::cout << "Manage time-based priority score adjustments." << std::endl;
  std::cout << "Example: priorisea time list" << std::endl;
  std::cout << std::endl;
  std::cout << "COMMAND list and interpretation:" << std::endl;
  std::cout << "  list: list time ranges" << std::endl;
  std::cout << "  get: get time range parameters" << std::endl;
  std::cout << "  add: add a time range" << std::endl;
  std::cout << "  edit: edit a time range" << std::endl;
  std::cout << "  delete: delete time ranges" << std::endl;
  return 0;
}

static const std::string LIST_USAGE_MSG =
    "Usage: priorisea time list [-h] [OPTIONS]";
static const std::string LIST_HELP_MSG =
    "Try 'priorisea time list -h' for more information.";

static int run_list(int argc, char *argv[]) {
  bool json_flag = false;

  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << LIST_USAGE_MSG << std::endl;
    std::cout << "List configured time ranges." << std::endl;
    std::cout << "Example: priorisea time list" << std::endl;
    std::cout << std::endl;
    std::cout << "OPTIONS list and interpretation:" << std::endl;
    std::cout << "  --json: output in JSON format" << std::endl;
    return 0;
  }

  for (int i = 0; i < argc; i++) {
    std::string arg = argv[i];

    if (arg == "--json") {
      json_flag = true;
    } else {
      std::cout << "Error: 'priorisea time list " << arg
                << "' is not a valid option" << std::endl;
      std::cout << LIST_USAGE_MSG << std::endl;
      std::cout << LIST_HELP_MSG << std::endl;
      return -1;
    }
  }

  // TODO: Retrieve the time list from the database
  std::vector<struct time> results = {
      {.name = "daily", .cron = "0 0 * * *", .duration = 3600, .priority = 10},
      {.name = "hourly", .cron = "0 * * * *", .duration = 60, .priority = 10}};

  if (json_flag) {
    auto obj = json_array();

    for (const auto &t : results) {
      json_array_append_new(obj, json_string(t.name.c_str()));
    }

    std::cout << json_dumps(obj, JSON_INDENT(2)) << std::endl;
    json_decref(obj);
  } else {
    std::string header_name = "Name";
    std::size_t name_width = header_name.size();

    for (const auto &t : results) {
      name_width = std::max(name_width, t.name.size());
    }

    std::cout << std::left << std::setw(name_width) << header_name << '\n';
    std::cout << std::string(name_width, '-') << '\n';

    for (const auto &t : results) {
      std::cout << std::left << std::setw(name_width) << t.name << '\n';
    }

    std::cout << std::endl;
  }

  return 0;
}

static const std::string GET_USAGE_MSG =
    "Usage: priorisea time get [-h] [OPTIONS] NAME [NAME...]";
static const std::string GET_HELP_MSG =
    "Try 'priorisea time get -h' for more information.";

static int run_get(int argc, char *argv[]) {
  bool json_flag = false;
  std::vector<std::string> times;

  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << GET_USAGE_MSG << std::endl;
    std::cout << "Retrieve one or more time range configurations from the "
                 "database."
              << std::endl;
    std::cout << "Example: priorisea time get daily hourly" << std::endl;
    std::cout << std::endl;
    std::cout << "OPTIONS list and interpretation:" << std::endl;
    std::cout << "  --json: output in JSON format" << std::endl;
    return 0;
  }

  if (argc < 1) {
    std::cout << GET_USAGE_MSG << std::endl;
    std::cout << GET_HELP_MSG << std::endl;
    return -1;
  }

  for (int i = 0; i < argc; i++) {
    std::string arg = argv[i];

    if (arg == "--json") {
      json_flag = true;
    } else if (!arg.empty() && arg[0] == '-') {
      std::cout << "Error: 'priorisea time get " << arg
                << "' is not a valid option" << std::endl;
      std::cout << GET_USAGE_MSG << std::endl;
      std::cout << GET_HELP_MSG << std::endl;
      return -1;
    } else {
      times.push_back(arg);
    }
  }

  if (times.empty()) {
    std::cout << "Error: 'priorisea time get ...'" << std::endl;
    std::cout << "  time range name is missing" << std::endl;
    std::cout << GET_USAGE_MSG << std::endl;
    std::cout << GET_HELP_MSG << std::endl;
    return -1;
  }

  // TODO: Retrieve the time ranges from the database
  std::vector<struct time> results = {
      {.name = "daily", .cron = "0 0 * * *", .duration = 3600, .priority = 10},
      {.name = "hourly", .cron = "0 * * * *", .duration = 60, .priority = 10}};

  if (json_flag) {
    auto obj = json_object();

    for (const auto &f : results) {
      auto expr = json_object();
      json_object_set_new(expr, "cron", json_string(f.cron.c_str()));
      json_object_set_new(expr, "duration", json_integer(f.duration));
      json_object_set_new(expr, "priority", json_real(f.priority));
      json_object_set_new(obj, f.name.c_str(), expr);
    }

    std::cout << json_dumps(obj, JSON_INDENT(2)) << std::endl;
    json_decref(obj);
  } else {
    std::string header_name = "Name";
    std::string header_cron = "Cron";
    std::string header_duration = "Duration";
    std::string header_priority = "Priority";

    std::size_t name_width = header_name.size();
    std::size_t cron_width = header_cron.size();
    std::size_t duration_width = header_duration.size();
    std::size_t priority_width = header_priority.size();

    for (const auto &f : results) {
      name_width = std::max(name_width, f.name.size());
      cron_width = std::max(cron_width, f.cron.size());
      duration_width =
          std::max(duration_width, std::to_string(f.duration).size());
      priority_width =
          std::max(priority_width, std::to_string(f.priority).size());
    }

    std::cout << std::left << std::setw(name_width) << header_name << " | "
              << std::setw(cron_width) << header_cron << " | "
              << std::setw(duration_width) << header_duration << " | "
              << std::setw(priority_width) << header_priority << '\n';

    std::cout << std::string(name_width, '-') << "-+-"
              << std::string(cron_width, '-') << "-+-"
              << std::string(duration_width, '-') << "-+-"
              << std::string(priority_width, '-') << '\n';

    for (const auto &f : results) {
      std::cout << std::left << std::setw(name_width) << f.name << " | "
                << std::setw(cron_width) << f.cron << " | "
                << std::setw(duration_width) << f.duration << " | "
                << std::setw(priority_width) << f.priority << '\n';
    }

    std::cout << std::endl;
  }

  return 0;
}

static const std::string ADD_USAGE_MSG =
    "Usage: priorisea time add [-h] NAME \"<cron_expression>\" DURATION "
    "PRIORITY";
static const std::string ADD_HELP_MSG =
    "Try 'priorisea time add -h' for more information.";

static int run_add(int argc, char *argv[]) {
  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << ADD_USAGE_MSG << std::endl;
    std::cout << "Add a new time range for priority score adjustment."
              << std::endl;
    std::cout << "Example: priorisea time add daily \"0 0 * * *\" 3600 10"
              << std::endl;
    return 0;
  }

  if (argc != 4) {
    std::cout << ADD_USAGE_MSG << std::endl;
    std::cout << ADD_HELP_MSG << std::endl;
    return -1;
  }

  std::string range_name = argv[0];
  std::string cron_expression = argv[1];

  int duration = 0;
  double priority = 0;

  if (!parse_int(argv[2], duration)) {
    std::cout << "Error: 'priorisea time add ...'" << std::endl;
    std::cout << "  duration `" << argv[2] << "` is not a valid integer"
              << std::endl;
    std::cout << ADD_USAGE_MSG << std::endl;
    std::cout << ADD_HELP_MSG << std::endl;
    return -1;
  }

  if (duration < 0) {
    std::cout << "Error: 'priorisea time add ...'" << std::endl;
    std::cout << "  duration `" << argv[2] << "` is not a positive integer"
              << std::endl;
    std::cout << ADD_USAGE_MSG << std::endl;
    std::cout << ADD_HELP_MSG << std::endl;
    return -1;
  }

  if (!parse_double(argv[3], priority)) {
    std::cout << "Error: 'priorisea time add ...'" << std::endl;
    std::cout << "  priority `" << argv[3] << "` is not a valid double"
              << std::endl;
    std::cout << ADD_USAGE_MSG << std::endl;
    std::cout << ADD_HELP_MSG << std::endl;
    return -1;
  }

  // TODO: Add the time range to the database
  return 0;
}

static const std::string EDIT_USAGE_MSG =
    "Usage: priorisea time edit [-h] NAME [--cron CRON] "
    "[--duration DURATION] [--priority PRIORITY]";
static const std::string EDIT_HELP_MSG =
    "Try 'priorisea time edit -h' for more information.";

static int run_edit(int argc, char *argv[]) {
  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << EDIT_USAGE_MSG << std::endl;
    std::cout << "Edit an existing time range." << std::endl;
    std::cout << "Example: priorisea time edit daily --duration 7200"
              << std::endl;
    std::cout << "Example: priorisea time edit daily --cron \"0 12 * * *\" "
                 "--priority 5"
              << std::endl;
    std::cout << std::endl;
    std::cout << "OPTIONS list and interpretation:" << std::endl;
    std::cout << "  --cron: update the cron expression" << std::endl;
    std::cout << "  --duration: update the duration" << std::endl;
    std::cout << "  --priority: update the priority" << std::endl;
    return 0;
  }

  if (argc < 3) {
    std::cout << EDIT_USAGE_MSG << std::endl;
    std::cout << EDIT_HELP_MSG << std::endl;
    return -1;
  }

  std::string range_name = argv[0];

  // TODO: verify if the range name exists in the database

  std::string cron_expression;
  std::string duration_str;
  std::string priority_str;

  for (int i = 1; i < argc; i += 2) {
    std::string field = argv[i];

    if (i + 1 >= argc) {
      std::cout << "Error: 'priorisea time edit " << field << "'"
                << std::endl;
      std::cout << "  value is missing" << std::endl;
      std::cout << EDIT_USAGE_MSG << std::endl;
      std::cout << EDIT_HELP_MSG << std::endl;
      return -1;
    }

    std::string value = argv[i + 1];

    if (field == "--cron") {
      cron_expression = value;
    } else if (field == "--duration") {
      int duration = 0;

      if (!parse_int(value, duration)) {
        std::cout << "Error: 'priorisea time edit ...'" << std::endl;
        std::cout << "  duration `" << value << "` is not a valid integer"
                  << std::endl;
        std::cout << EDIT_USAGE_MSG << std::endl;
        std::cout << EDIT_HELP_MSG << std::endl;
        return -1;
      }

      if (duration < 0) {
        std::cout << "Error: 'priorisea time edit ...'" << std::endl;
        std::cout << "  duration `" << value
                  << "` is not a positive integer" << std::endl;
        std::cout << EDIT_USAGE_MSG << std::endl;
        std::cout << EDIT_HELP_MSG << std::endl;
        return -1;
      }

      duration_str = value;
    } else if (field == "--priority") {
      double priority = 0;

      if (!parse_double(value, priority)) {
        std::cout << "Error: 'priorisea time edit ...'" << std::endl;
        std::cout << "  priority `" << value << "` is not a valid double"
                  << std::endl;
        std::cout << EDIT_USAGE_MSG << std::endl;
        std::cout << EDIT_HELP_MSG << std::endl;
        return -1;
      }

      priority_str = value;
    } else {
      std::cout << "Error: 'priorisea time edit " << field
                << "' is not a valid option" << std::endl;
      std::cout << EDIT_USAGE_MSG << std::endl;
      std::cout << EDIT_HELP_MSG << std::endl;
      return -1;
    }
  }

  if (cron_expression.empty() && duration_str.empty() && priority_str.empty()) {
    std::cout << "Error: 'priorisea time edit ...'" << std::endl;
    std::cout << "  field to edit is missing" << std::endl;
    std::cout << EDIT_USAGE_MSG << std::endl;
    std::cout << EDIT_HELP_MSG << std::endl;
    return -1;
  }

  // TODO: edit the range in the database
  return 0;
}

static const std::string DELETE_USAGE_MSG =
    "Usage: priorisea time delete [-h] NAME [NAME...]";
static const std::string DELETE_HELP_MSG =
    "Try 'priorisea time delete -h' for more information.";

static int run_delete(int argc, char *argv[]) {
  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << DELETE_USAGE_MSG << std::endl;
    std::cout << "Delete one or more time ranges." << std::endl;
    std::cout << "Example: priorisea time delete daily hourly" << std::endl;
    return 0;
  }

  if (argc < 1) {
    std::cout << DELETE_USAGE_MSG << std::endl;
    std::cout << DELETE_HELP_MSG << std::endl;
    return -1;
  }

  for (int i = 0; i < argc; i++) {
    std::string range_name = argv[i];

    if (!range_name.empty() && range_name[0] == '-') {
      std::cout << "Error: 'priorisea time delete " << range_name
                << "' is not a valid option" << std::endl;
      std::cout << DELETE_USAGE_MSG << std::endl;
      std::cout << DELETE_HELP_MSG << std::endl;
      return -1;
    }

    // TODO: verify if the range name exists in the database
    // TODO: delete the range from the database
  }

  return 0;
}

int run_time(int argc, char *argv[]) {
  if (argc < 1) {
    std::cout << USAGE_MSG << std::endl;
    std::cout << HELP_MSG << std::endl;
    return -1;
  }

  std::string cmd = argv[0];

  if (cmd == "-h") {
    return run_help(argc - 1, argv + 1);
  } else if (cmd == "list") {
    return run_list(argc - 1, argv + 1);
  } else if (cmd == "get") {
    return run_get(argc - 1, argv + 1);
  } else if (cmd == "add") {
    return run_add(argc - 1, argv + 1);
  } else if (cmd == "edit") {
    return run_edit(argc - 1, argv + 1);
  } else if (cmd == "delete") {
    return run_delete(argc - 1, argv + 1);
  } else {
    std::cout << "Error: 'priorisea time " << cmd
              << "' is not a valid command" << std::endl;
    std::cout << USAGE_MSG << std::endl;
    std::cout << HELP_MSG << std::endl;
    return -1;
  }
}
