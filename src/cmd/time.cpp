// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "time.hpp"
#include "common/util.hpp"
#include "croncpp.h"
#include "lib/table.hpp"
#include <flux/core.h>
#include <iostream>
#include <jansson.h>
#include <lib/kvs.hpp>
#include <sstream>
#include <string.h>
#include <string>
#include <vector>

struct timeframe {
  std::string name;
  std::string cron;
  int duration;
  double priority;
};

////

static std::string timeframe_to_str(timeframe time) {
  std::stringstream ss;
  ss << time.name << "," << time.cron << "," << time.duration << ","
     << time.priority;
  return ss.str();
}

struct validate_cron_res {
  bool ok;
  std::string message;
};
static validate_cron_res validate_cron(const std::string &expr,
                                       std::string &error) {
  validate_cron_res res;
  try {
    cron::make_cron(expr);
    res.ok = true;
    return res;
  } catch (cron::bad_cronexpr const &ex) {
    res.ok = false;
    res.message = ex.what();
    return res;
  }
}

struct validate_timeframe_res {
  bool ok;
  std::string message;
};
static validate_timeframe_res validate_timeframe(timeframe time) {
  validate_timeframe_res res;
  if (time.name.empty()) {
    res.ok = false;
    res.message = "time name is missing.";
    return res;
  }
  auto cron_res = validate_cron(time.cron, res.message);
  if (!cron_res.ok) {
    res.ok = false;
    res.message = cron_res.message;
    return res;
  }
  if (time.duration < 0) {
    res.ok = false;
    res.message = "duration is not a positive integer";
    return res;
  }
  res.ok = true;
  return res;
}

struct timeframe_from_str_res {
  bool ok;
  std::string message;
  timeframe timeframe_obj;
};
static timeframe_from_str_res timeframe_from_str(const std::string &str) {
  timeframe_from_str_res res;

  std::stringstream ss(str);
  std::string t;
  if (!getline(ss, t, ',')) {
    res.ok = false;
    res.message = "name is missing";
    return res;
  }
  res.timeframe_obj.name = t;

  if (!getline(ss, t, ',')) {
    res.ok = false;
    res.message = "cron is missing";
    return res;
  }
  res.timeframe_obj.cron = t;

  if (!getline(ss, t, ',')) {
    res.ok = false;
    res.message = "duration is missing";
    return res;
  }
  int duration = 0;
  if (!parse_int(t, duration)) {
    res.ok = false;
    res.message = "duration `" + t + "` is not a valid integer";
    return res;
  }
  res.timeframe_obj.duration = duration;

  if (!getline(ss, t, ',')) {
    res.ok = false;
    res.message = "priority is missing";
    return res;
  }
  double priority = 0;
  if (!parse_double(t, priority)) {
    res.ok = false;
    res.message = "priority `" + t + "` is not a valid double";
    return res;
  }
  res.timeframe_obj.priority = priority;

  auto validate_res = validate_timeframe(res.timeframe_obj);
  if (!validate_res.ok) {
    res.ok = false;
    res.message = validate_res.message;
    return res;
  }

  res.ok = true;
  return res;
}

struct store_timeframe_res {
  bool ok;
  std::string message;
};
static store_timeframe_res store_timeframe(timeframe time) {
  store_timeframe_res res;

  std::string timeframe_str = timeframe_to_str(time);

  auto txn = flux_kvs_txn_create();
  if (!txn) {
    res.ok = false;
    res.message = "Failed to create KVS transaction.";
    return res;
  }
  if (flux_kvs_txn_put(txn, 0, ("priorisea.timeframe." + time.name).c_str(),
                       timeframe_str.c_str()) < 0) {
    res.ok = false;
    res.message = "Failed to store time in KVS transaction.";
    return res;
  }
  auto commit_res = kvs_commit(txn);
  if (commit_res != KVS_ERR_SUCCESS) {
    res.ok = false;
    res.message = "Failed to commit time to KVS.";
    return res;
  }
  res.ok = true;
  return res;
}

struct get_timeframe_res {
  bool ok;
  std::string message;
  std::string name;
  timeframe timeframe_obj;
};
struct get_timeframes_res {
  bool ok;
  std::string message;
  std::vector<get_timeframe_res> res;
};
static get_timeframes_res get_timeframes(std::vector<std::string> names) {
  get_timeframes_res res;
  res.ok = false;

  std::vector<std::string> keys;
  for (auto timeframe_name : names) {
    keys.push_back("priorisea.timeframe." + timeframe_name);
  }

  auto lookup_res = kvs_lookup(keys);
  if (lookup_res.err != KVS_ERR_SUCCESS) {
    res.message = "failed to lookup timeframes in KVS. Unknown error";
    return res;
  }

  for (const auto &lookup_key_res : lookup_res.res) {
    get_timeframe_res timeframe_res;
    timeframe_res.ok = false;

    size_t pos = lookup_key_res.key.rfind('.');
    std::string timeframe_name = (pos == std::string::npos)
                                     ? lookup_key_res.key
                                     : lookup_key_res.key.substr(pos + 1);
    timeframe_res.name = timeframe_name;

    if (lookup_key_res.err != KVS_ERR_SUCCESS) {
      timeframe_res.message =
          "Failed to lookup timeframe '" + timeframe_name + "'";
      res.res.push_back(timeframe_res);
      continue;
    }

    auto from_res = timeframe_from_str(lookup_key_res.value);
    if (!from_res.ok) {
      timeframe_res.message = from_res.message;
      res.res.push_back(timeframe_res);
      continue;
    }

    timeframe_res.ok = true;
    timeframe_res.timeframe_obj = from_res.timeframe_obj;
    res.res.push_back(timeframe_res);
  }

  res.ok = true;
  return res;
}

struct del_timeframes_res {
  bool ok;
  std::string message;
};
static del_timeframes_res del_timeframes(std::vector<std::string> names) {
  del_timeframes_res res;
  std::vector<std::string> keys;
  for (auto name : names) {
    keys.push_back("priorisea.timeframe." + name);
  }
  auto remove_res = kvs_remove(keys);
  if (remove_res != KVS_ERR_SUCCESS) {
    res.ok = false;
    res.message = "failed to delete times";
    return res;
  }
  res.ok = true;
  return res;
}

////

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

  std::cerr << "Error: 'priorisea time list' is not yet implemented"
            << std::endl;
  return -1;

  std::vector<timeframe> results = {
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
    print_table(results, {{"Name", [](auto &value, auto index) -> auto {
                             return value.name;
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
