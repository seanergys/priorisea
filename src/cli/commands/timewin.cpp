#include "src/cli/commands/timewin.hpp"
#include "src/cli/endpoint/flux/timewin.hpp"
#include "src/cli/util.hpp"
#include "src/lib/error.hpp"
#include "src/lib/timewin.hpp"
#include <jansson.h>
#include <memory>
#include <string>
#include <vector>

static void print_timewin(const ps_timewin_t &timewin, bool json) {
  if (json) {
    auto obj = ps_timewin_to_json(timewin);
    char *dump = json_dumps(obj, JSON_INDENT(2));
    if (dump) {
      std::cout << dump << std::endl;
      free(dump);
    }
    json_decref(obj);
    return;
  }
  ps_print_table_v(
      std::vector<ps_timewin_t>{timewin},
      {{"Name", [](auto &t, auto) { return t.name; }},
       {"Cron", [](auto &t, auto) { return t.cron; }},
       {"Duration", [](auto &t, auto) { return std::to_string(t.duration); }},
       {"Score", [](auto &t, auto) { return std::to_string(t.score); }}});
}

static void print_timewins(const std::vector<ps_timewin_t> &timewins,
                           bool json) {
  if (json) {
    auto arr = json_array();
    for (const auto &timewin : timewins) {
      json_array_append_new(arr, ps_timewin_to_json(timewin));
    }
    char *dump = json_dumps(arr, JSON_INDENT(2));
    if (dump) {
      std::cout << dump << std::endl;
      free(dump);
    }
    json_decref(arr);
    return;
  }
  ps_print_table_v(
      timewins,
      {{"Name", [](auto &t, auto) { return t.name; }},
       {"Cron", [](auto &t, auto) { return t.cron; }},
       {"Duration", [](auto &t, auto) { return std::to_string(t.duration); }},
       {"Score", [](auto &t, auto) { return std::to_string(t.score); }}});
}

static void register_get_command(CLI::App *app) {
  auto *get_cmd = app->add_subcommand("get", "Get a time window");
  get_cmd->set_help_flag("-h,--help", "Show help for get command");
  struct options {
    std::string name;
    bool json;
  };
  auto opts = std::make_shared<options>();
  get_cmd->add_option("name", opts->name, "Time window name")->required(true);
  get_cmd->add_flag("-j,--json", opts->json, "Output JSON");
  get_cmd->callback([opts] {
    auto get_res = ps_cli_endpoint_flux_timewin_get(opts->name);
    if (!ps_is_ok(get_res)) {
      ps_exit_error(get_res);
    }
    print_timewin(ps_unwrap(get_res), opts->json);
  });
}

static void register_list_command(CLI::App *app) {
  auto *list_cmd = app->add_subcommand("list", "List time windows");
  list_cmd->set_help_flag("-h,--help", "Show help for list command");
  struct options {
    std::vector<std::string> names;
    bool json;
  };
  auto opts = std::make_shared<options>();
  list_cmd->add_option("names", opts->names, "Time window names (optional)");
  list_cmd->add_flag("-j,--json", opts->json, "Output JSON");
  list_cmd->callback([opts] {
    auto list_res = ps_cli_endpoint_flux_timewin_list(opts->names);
    if (!ps_is_ok(list_res)) {
      ps_exit_error(list_res);
    }
    print_timewins(ps_unwrap(list_res), opts->json);
  });
}

static void register_update_command(CLI::App *app) {
  auto *update_cmd = app->add_subcommand("update", "Update a time window");
  update_cmd->set_help_flag("-h,--help", "Show help for update command");
  struct options {
    std::string name;
    std::string cron;
    int duration;
    double score;
  };
  auto opts = std::make_shared<options>();
  update_cmd->add_option("name", opts->name, "Time window name")
      ->required(true);
  update_cmd->add_option("-c,--cron", opts->cron, "Cron expression")
      ->required(true);
  update_cmd->add_option("-d,--duration", opts->duration, "Duration in seconds")
      ->required(true);
  update_cmd->add_option("-s,--score", opts->score, "Score")->required(true);
  update_cmd->callback([opts] {
    ps_timewin_t timewin{.name = opts->name,
                         .cron = opts->cron,
                         .duration = opts->duration,
                         .score = opts->score};
    auto res = ps_cli_endpoint_flux_timewin_update(timewin);
    if (!ps_is_ok(res)) {
      ps_exit_error(res);
    }
  });
}

static void register_create_command(CLI::App *app) {
  auto *create_cmd = app->add_subcommand("create", "Create a time window");
  create_cmd->set_help_flag("-h,--help", "Show help for create command");
  struct options {
    std::string name;
    std::string cron;
    int duration;
    double score;
  };
  auto opts = std::make_shared<options>();
  create_cmd->add_option("name", opts->name, "Time window name")
      ->required(true);
  create_cmd->add_option("-c,--cron", opts->cron, "Cron expression")
      ->required(true);
  create_cmd->add_option("-d,--duration", opts->duration, "Duration in seconds")
      ->required(true);
  create_cmd->add_option("-s,--score", opts->score, "Score")->required(true);
  create_cmd->callback([opts] {
    ps_timewin_t timewin{.name = opts->name,
                         .cron = opts->cron,
                         .duration = opts->duration,
                         .score = opts->score};
    auto res = ps_cli_endpoint_flux_timewin_create(timewin);
    if (!ps_is_ok(res)) {
      ps_exit_error(res);
    }
  });
}

static void register_delete_command(CLI::App *app) {
  auto *delete_cmd = app->add_subcommand("delete", "Delete time windows");
  delete_cmd->set_help_flag("-h,--help", "Show help for delete command");
  struct options {
    std::vector<std::string> names;
  };
  auto opts = std::make_shared<options>();
  delete_cmd->add_option("names", opts->names, "Time window names")
      ->required(true);
  delete_cmd->callback([opts] {
    auto res = ps_cli_endpoint_flux_timewin_delete(opts->names);
    if (!ps_is_ok(res)) {
      ps_exit_error(res);
    }
  });
}

void register_timewin_commands(CLI::App *app) {
  auto *timewin_cmd = app->add_subcommand("timewin", "Manage time windows");
  timewin_cmd->require_subcommand(true);
  timewin_cmd->set_help_flag("-h,--help", "Show help for timewin commands");
  register_get_command(timewin_cmd);
  register_list_command(timewin_cmd);
  register_update_command(timewin_cmd);
  register_create_command(timewin_cmd);
  register_delete_command(timewin_cmd);
}
