// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "time.hpp"
#include "lib/time.hpp"
#include "lib/util.hpp"
#include <flux/core.h>
#include <iostream>
#include <jansson.h>
#include <string.h>
#include <string>
#include <vector>

static const std::string USAGE_MSG = "Usage: priorisea time [-h] COMMAND ARGS";
static const std::string TRY_HELP_MSG =
    "Try 'priorisea time -h' for more information.";
static const std::string HELP_MSG = USAGE_MSG + R"(

Manage time-based priority score adjustments.
Example: priorisea time list

COMMAND list and interpretation:
  list: list time ranges
  get: get time range parameters
  add: add a time range
  edit: edit a time range
  delete: delete time ranges)";

static const std::string LIST_USAGE_MSG =
    "Usage: priorisea time list [-h] [OPTIONS]";
static const std::string LIST_TRY_HELP_MSG =
    "Try 'priorisea time list -h' for more information.";
static const std::string LIST_HELP_MSG = LIST_USAGE_MSG + R"(

List configured time ranges.
Example: priorisea time list

OPTIONS list and interpretation:
  --json: output in JSON format)";

static int run_list(int argc, char *argv[]) {
  bool json_flag = false;

  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << LIST_HELP_MSG << std::endl;
    return 0;
  }

  for (int i = 0; i < argc; i++) {
    std::string arg = argv[i];

    if (arg == "--json") {
      json_flag = true;
    } else {
      std::cerr << "Error: 'priorisea time list " << arg
                << "' is not a valid option" << '\n'
                << LIST_USAGE_MSG << '\n'
                << LIST_TRY_HELP_MSG << std::endl;
      return -1;
    }
  }

  auto list_res = list_timeframes();
  if (!list_res.ok) {
    std::cerr << "Error: 'priorisea time list ...'" << '\n'
              << "  " << list_res.message << '\n'
              << LIST_USAGE_MSG << '\n'
              << LIST_TRY_HELP_MSG << std::endl;
    return -1;
  }
  std::vector<timeframe> timeframes;
  for (const auto &get_timeframe_res : list_res.res) {
    if (get_timeframe_res.ok) {
      timeframes.push_back(get_timeframe_res.timeframe_obj);
    } else {
      std::cerr << "Error: " << list_res.message << std::endl;
    }
  }

  if (json_flag) {
    auto obj = json_object();

    for (const auto &f : timeframes) {
      auto expr = json_object();
      json_object_set_new(expr, "cron", json_string(f.cron.c_str()));
      json_object_set_new(expr, "duration", json_integer(f.duration));
      json_object_set_new(expr, "priority", json_real(f.priority));
      json_object_set_new(obj, f.name.c_str(), expr);
    }

    std::cout << json_dumps(obj, JSON_INDENT(2)) << std::endl;
    json_decref(obj);
  } else {
    print_table(
        timeframes,
        {{"Name", [](auto &value, auto index) -> auto { return value.name; }},
         {"Cron", [](auto &value, auto index) -> auto { return value.cron; }},
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

static const std::string GET_USAGE_MSG =
    "Usage: priorisea time get [-h] [OPTIONS] NAME [NAME...]";
static const std::string GET_TRY_HELP_MSG =
    "Try 'priorisea time get -h' for more information.";
static const std::string GET_HELP_MSG = GET_USAGE_MSG + R"(

Retrieve one or more time range configurations from the database.
Example: priorisea time get daily hourly

OPTIONS list and interpretation:
  --json: output in JSON format)";

static int run_get(int argc, char *argv[]) {
  bool json_flag = false;
  std::vector<std::string> timeframes_names;

  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << GET_HELP_MSG << std::endl;
    return 0;
  }

  for (int i = 0; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--json") {
      json_flag = true;
    } else {
      timeframes_names.push_back(arg);
    }
  }

  if (timeframes_names.empty()) {
    std::cerr << GET_USAGE_MSG << '\n' << GET_TRY_HELP_MSG << std::endl;
    return -1;
  }

  auto get_res = get_timeframes(timeframes_names);
  if (!get_res.ok) {
    std::cerr << "Error: 'priorisea time get ...'" << '\n'
              << "  " << get_res.message << '\n'
              << GET_USAGE_MSG << '\n'
              << GET_TRY_HELP_MSG << std::endl;
    return -1;
  }
  std::vector<timeframe> timeframes;
  for (const auto &get_timeframe_res : get_res.res) {
    if (get_timeframe_res.ok) {
      timeframes.push_back(get_timeframe_res.timeframe_obj);
    } else {
      std::cerr << "Error: " << get_res.message << std::endl;
    }
  }

  if (json_flag) {
    auto obj = json_object();

    for (const auto &f : timeframes) {
      auto expr = json_object();
      json_object_set_new(expr, "cron", json_string(f.cron.c_str()));
      json_object_set_new(expr, "duration", json_integer(f.duration));
      json_object_set_new(expr, "priority", json_real(f.priority));
      json_object_set_new(obj, f.name.c_str(), expr);
    }

    std::cout << json_dumps(obj, JSON_INDENT(2)) << std::endl;
    json_decref(obj);
  } else {
    print_table(
        timeframes,
        {{"Name", [](auto &value, auto index) -> auto { return value.name; }},
         {"Cron", [](auto &value, auto index) -> auto { return value.cron; }},
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

static const std::string ADD_USAGE_MSG =
    "Usage: priorisea time add [-h] NAME \"CRON\" DURATION "
    "PRIORITY";
static const std::string ADD_TRY_HELP_MSG =
    "Try 'priorisea time add -h' for more information.";
static const std::string ADD_HELP_MSG = ADD_USAGE_MSG + R"(

Add a new time range for priority score adjustment.
Example: priorisea time add daily \"0 0 * * *\" 3600 10")";

static int run_add(int argc, char *argv[]) {
  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << ADD_HELP_MSG << std::endl;
    return 0;
  }

  if (argc != 4) {
    std::cerr << ADD_USAGE_MSG << '\n' << ADD_TRY_HELP_MSG << std::endl;
    return -1;
  }

  std::string timeframe_str =
      std::string(argv[0]) + "," + argv[1] + "," + argv[2] + "," + argv[3];
  auto parse_res = timeframe_from_str(timeframe_str);
  if (!parse_res.ok) {
    std::cerr << "Error: 'priorisea time add ...'" << '\n'
              << "  " << parse_res.message << '\n'
              << ADD_USAGE_MSG << '\n'
              << ADD_TRY_HELP_MSG << std::endl;
    return -1;
  }
  timeframe tf = parse_res.timeframe_obj;

  auto store_res = store_timeframe(tf);
  if (!store_res.ok) {
    std::cerr << "Error: 'priorisea time add ...'" << '\n'
              << "  " << store_res.message << '\n'
              << ADD_USAGE_MSG << '\n'
              << ADD_TRY_HELP_MSG << std::endl;
    return -1;
  }
  return 0;
}

static const std::string EDIT_USAGE_MSG =
    "Usage: priorisea time edit [-h] NAME [--cron CRON] "
    "[--duration DURATION] [--priority PRIORITY]";
static const std::string EDIT_TRY_HELP_MSG =
    "Try 'priorisea time edit -h' for more information.";
static const std::string EDIT_HELP_MSG = EDIT_USAGE_MSG + R"(

Edit an existing time range.
Example: priorisea time edit daily --duration 7200
Example: priorisea time edit daily --cron "0 12 * * *" --priority 5

OPTIONS list and interpretation:
  --cron: update the cron expression
  --duration: update the duration
  --priority: update the priority)";

static int run_edit(int argc, char *argv[]) {
  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << EDIT_HELP_MSG << std::endl;
    return 0;
  }

  if (argc < 3) {
    std::cerr << EDIT_USAGE_MSG << '\n' << EDIT_TRY_HELP_MSG << std::endl;
    return -1;
  }

  std::string range_name = argv[0];

  auto get_res = get_timeframes({range_name});
  if (!get_res.ok) {
    std::cerr << "Error: 'priorisea time edit " << range_name << "'" << '\n'
              << "  " << get_res.message << '\n'
              << EDIT_USAGE_MSG << '\n'
              << EDIT_TRY_HELP_MSG << std::endl;
    return -1;
  }

  auto get_timeframe_res = get_res.res[0];
  if (!get_timeframe_res.ok) {
    std::cerr << "Error: 'priorisea time edit " << range_name << "'" << '\n'
              << "  " << get_timeframe_res.message << '\n'
              << EDIT_USAGE_MSG << '\n'
              << EDIT_TRY_HELP_MSG << std::endl;
    return -1;
  }
  auto timeframe_obj = get_timeframe_res.timeframe_obj;

  std::string cron_expression = timeframe_obj.cron;
  std::string duration_str = std::to_string(timeframe_obj.duration);
  std::string priority_str = std::to_string(timeframe_obj.priority);

  for (int i = 1; i < argc; i += 2) {
    std::string field = argv[i];
    if (i + 1 >= argc) {
      std::cerr << "Error: 'priorisea time edit " << field << "'" << '\n'
                << "  value is missing" << '\n'
                << EDIT_USAGE_MSG << '\n'
                << EDIT_TRY_HELP_MSG << std::endl;
      return -1;
    }

    std::string value = argv[i + 1];
    if (field == "--cron") {
      cron_expression = value;
    } else if (field == "--duration") {
      duration_str = value;
    } else if (field == "--priority") {
      priority_str = value;
    } else {
      std::cerr << "Error: 'priorisea time edit " << field
                << "' is not a valid option" << '\n'
                << EDIT_USAGE_MSG << '\n'
                << EDIT_TRY_HELP_MSG << std::endl;
      return -1;
    }
  }

  if (cron_expression.empty() && duration_str.empty() && priority_str.empty()) {
    std::cerr << "Error: 'priorisea time edit ...'" << '\n'
              << "  field to edit is missing" << '\n'
              << EDIT_USAGE_MSG << '\n'
              << EDIT_TRY_HELP_MSG << std::endl;
    return -1;
  }

  auto timeframe_str = timeframe_obj.name + "," + cron_expression + "," +
                       duration_str + "," + priority_str;
  auto parse_res = timeframe_from_str(timeframe_str);
  if (!parse_res.ok) {
    std::cerr << "Error: 'priorisea time edit ...'" << '\n'
              << "  " << parse_res.message << '\n'
              << EDIT_USAGE_MSG << '\n'
              << EDIT_TRY_HELP_MSG << std::endl;
    return -1;
  }
  auto store_res = store_timeframe(parse_res.timeframe_obj);
  if (!store_res.ok) {
    std::cerr << "Error: 'priorisea time edit ...'" << '\n'
              << "  " << store_res.message << '\n'
              << EDIT_USAGE_MSG << '\n'
              << EDIT_TRY_HELP_MSG << std::endl;
    return -1;
  }

  return 0;
}

static const std::string DELETE_USAGE_MSG =
    "Usage: priorisea time delete [-h] NAME [NAME...]";
static const std::string DELETE_TRY_HELP_MSG =
    "Try 'priorisea time delete -h' for more information.";
static const std::string DELETE_HELP_MSG = DELETE_USAGE_MSG + R"(

Delete one or more time ranges.
"Example: priorisea time delete daily hourly)";

static int run_delete(int argc, char *argv[]) {
  if (argc > 0 && strcmp(argv[0], "-h") == 0) {
    std::cout << DELETE_HELP_MSG << std::endl;
    return 0;
  }

  if (argc < 1) {
    std::cerr << DELETE_USAGE_MSG << '\n' << DELETE_TRY_HELP_MSG << std::endl;
    return -1;
  }

  for (int i = 0; i < argc; i++) {
    std::string range_name = argv[i];

    if (!range_name.empty() && range_name[0] == '-') {
      std::cerr << "Error: 'priorisea time delete " << range_name
                << "' is not a valid option" << '\n'
                << DELETE_USAGE_MSG << '\n'
                << DELETE_TRY_HELP_MSG << std::endl;
      return -1;
    }

    auto del_res = del_timeframes(std::vector<std::string>{range_name});
    if (!del_res.ok) {
      std::cout << "Error: " << del_res.message << std::endl;
      return -1;
    }
  }

  return 0;
}

int run_time(int argc, char *argv[]) {
  if (argc < 1) {
    std::cout << USAGE_MSG << '\n' << TRY_HELP_MSG << std::endl;
    return -1;
  }

  std::string cmd = argv[0];

  if (cmd == "-h") {
    std::cout << HELP_MSG << std::endl;
    return 0;
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
    std::cout << "Error: 'priorisea time " << cmd << "' is not a valid command"
              << '\n'
              << USAGE_MSG << '\n'
              << TRY_HELP_MSG << std::endl;
    return -1;
  }
}
