#include "src/cli/commands/user.hpp"
#include "src/cli/endpoint/flux/user.hpp"
#include "src/cli/util.hpp"
#include "src/lib/error.hpp"
#include "src/lib/user.hpp"
#include "src/lib/util.hpp"
#include <jansson.h>
#include <memory>
#include <string>
#include <sys/types.h>
#include <vector>

// Human-facing display name for a user
static std::string display_username(const ps_user_t &user) {
  std::string name;
  if (username_from_uid(user.userid, name)) {
    return name;
  }
  return std::to_string(static_cast<uint64_t>(user.userid));
}

static void print_user(const ps_user_t &user, bool json) {
  if (json) {
    auto obj = ps_user_to_json(user);
    char *dump = json_dumps(obj, JSON_INDENT(2));
    if (dump) {
      std::cout << dump << std::endl;
      free(dump);
    }
    json_decref(obj);
    return;
  }
  ps_print_table_v(
      std::vector<ps_user_t>{user},
      {{"Username", [](auto &u, auto) { return display_username(u); }},
       {"User ID",
        [](auto &u, auto) {
          return std::to_string(static_cast<uint64_t>(u.userid));
        }},
       {"Score", [](auto &u, auto) { return std::to_string(u.score); }}});
}

static void print_users(const std::vector<ps_user_t> &users, bool json) {
  if (json) {
    auto arr = json_array();
    for (const auto &user : users) {
      json_array_append_new(arr, ps_user_to_json(user));
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
      users,
      {{"Username", [](auto &u, auto) { return display_username(u); }},
       {"User ID",
        [](auto &u, auto) {
          return std::to_string(static_cast<uint64_t>(u.userid));
        }},
       {"Score", [](auto &u, auto) { return std::to_string(u.score); }}});
}

static void register_get_command(CLI::App *app) {
  auto *get_cmd = app->add_subcommand("get", "Get a user");
  get_cmd->set_help_flag("-h,--help", "Show help for get command");
  struct options {
    std::string username;
    bool json;
  };
  auto opts = std::make_shared<options>();
  get_cmd->add_option("username", opts->username, "Username")->required(true);
  get_cmd->add_flag("-j,--json", opts->json, "Output JSON");
  get_cmd->callback([opts] {
    auto get_res = ps_cli_endpoint_flux_user_get(opts->username);
    if (!ps_is_ok(get_res)) {
      ps_exit_error(get_res);
    }
    print_user(ps_unwrap(get_res), opts->json);
  });
}

static void register_list_command(CLI::App *app) {
  auto *list_cmd = app->add_subcommand("list", "List users");
  list_cmd->set_help_flag("-h,--help", "Show help for list command");
  struct options {
    std::vector<std::string> usernames;
    bool json;
  };
  auto opts = std::make_shared<options>();
  list_cmd->add_option("usernames", opts->usernames, "Usernames (optional)");
  list_cmd->add_flag("-j,--json", opts->json, "Output JSON");
  list_cmd->callback([opts] {
    auto list_res = ps_cli_endpoint_flux_user_list(opts->usernames);
    if (!ps_is_ok(list_res)) {
      ps_exit_error(list_res);
    }
    print_users(ps_unwrap(list_res), opts->json);
  });
}

static void register_update_command(CLI::App *app) {
  auto *update_cmd = app->add_subcommand("update", "Update users score");
  update_cmd->set_help_flag("-h,--help", "Show help for update command");
  struct options {
    std::vector<std::string> usernames;
    bool all;
  };
  auto opts = std::make_shared<options>();
  update_cmd->add_option("usernames", opts->usernames, "Usernames (optional)");
  update_cmd->add_flag("-a,--all", opts->all, "Update all users");
  update_cmd->callback([opts] {
    auto res = ps_cli_endpoint_flux_user_update(opts->usernames, opts->all);
    if (!ps_is_ok(res)) {
      ps_exit_error(res);
    }
  });
}

void register_user_commands(CLI::App *app) {
  auto *user_cmd = app->add_subcommand("user", "Manage users");
  user_cmd->require_subcommand(true);
  user_cmd->set_help_flag("-h,--help", "Show help for user commands");
  register_get_command(user_cmd);
  register_list_command(user_cmd);
  register_update_command(user_cmd);
}
