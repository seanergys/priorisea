#include "src/cli/commands/formula.hpp"
#include "src/cli/endpoint/flux/formula.hpp"
#include "src/cli/util.hpp"
#include "src/lib/error.hpp"
#include "src/lib/formula.hpp"
#include "src/lib/util.hpp"
#include <jansson.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

static void print_formulas(const std::vector<ps_formula_t> &formulas,
                           bool json) {
  if (json) {
    auto arr = json_array();
    for (const auto &formula : formulas) {
      json_array_append_new(arr, ps_formula_to_json(formula));
    }
    char *dump = json_dumps(arr, JSON_INDENT(2));
    if (dump) {
      std::cout << dump << std::endl;
      free(dump);
    }
    json_decref(arr);
    return;
  }
  ps_print_table_v(formulas,
                   {{"Name", [](auto &f, auto) { return f.name; }},
                    {"Expression", [](auto &f, auto) { return f.expr; }},
                    {"Defaults", [](auto &value, auto index) -> auto {
                       std::stringstream ss;
                       bool first = true;
                       for (const auto &[name, value] : value.defaults) {
                         if (!first) {
                           ss << ", ";
                         } else {
                           first = false;
                         }
                         ss << name << "=" << value;
                       }
                       return ss.str();
                     }}});
}

static void register_get_command(CLI::App *app) {
  auto *get_cmd = app->add_subcommand("get", "Get a formula");
  get_cmd->set_help_flag("-h,--help", "Show help for get command");
  struct options {
    std::string name;
    bool json;
  };
  auto opts = std::make_shared<options>();
  get_cmd->add_option("name", opts->name, "Formula name")->required(true);
  get_cmd->add_flag("-j,--json", opts->json, "Output JSON");
  get_cmd->callback([opts] {
    auto get_res = ps_cli_endpoint_flux_formula_get(opts->name);
    if (!ps_is_ok(get_res)) {
      ps_exit_error(get_res);
    }
    ps_print_formula(ps_unwrap(get_res), opts->json);
  });
}

static void register_list_command(CLI::App *app) {
  auto *list_cmd = app->add_subcommand("list", "List formulas");
  list_cmd->set_help_flag("-h,--help", "Show help for list command");
  struct options {
    std::vector<std::string> names;
    bool json;
  };
  auto opts = std::make_shared<options>();
  list_cmd->add_option("names", opts->names, "Formula names (optional)");
  list_cmd->add_flag("-j,--json", opts->json, "Output JSON");
  list_cmd->callback([opts] {
    auto list_res = ps_cli_endpoint_flux_formula_list(opts->names);
    if (!ps_is_ok(list_res)) {
      ps_exit_error(list_res);
    }
    print_formulas(ps_unwrap(list_res), opts->json);
  });
}

static void register_update_command(CLI::App *app) {
  auto *update_cmd = app->add_subcommand("update", "Update a formula");
  update_cmd->footer(
      "Example:\n  priorisea formula update job_priority "
      "\"<var:prev_priority> * 2\" --default var:prev_priority 5");
  update_cmd->set_help_flag("-h,--help", "Show help for update command");
  struct options {
    std::string name;
    std::string expr;
    std::vector<std::string> defaults;
  };
  auto opts = std::make_shared<options>();
  update_cmd->add_option("name", opts->name, "Formula name")->required(true);
  update_cmd->add_option("expr", opts->expr, "Formula expression")
      ->required(true);
  update_cmd
      ->add_option("-d,--default", opts->defaults,
                   "Default value of a variable (repeatable)")
      ->type_size(2);
  update_cmd->callback([opts] {
    ps_formula_t formula{.name = opts->name, .expr = opts->expr};
    // Retrieve default values for variables
    for (size_t i = 0; i < opts->defaults.size(); i += 2) {
      auto default_name = opts->defaults[i];
      auto value_str = opts->defaults[i + 1];
      double value;
      if (!parse_double(value_str, value)) {
        ps_exit_error(ps_error(ps_field_type_mismatch_t{
            .field = "defaults[" + default_name + "]", .expected = "double"}));
      }

      formula.defaults[default_name] = value;
    }
    auto update_res = ps_cli_endpoint_flux_formula_update(formula);
    if (!ps_is_ok(update_res)) {
      ps_exit_error(update_res);
    }
  });
}

static void register_reset_command(CLI::App *app) {
  auto *reset_cmd = app->add_subcommand("reset", "Reset formulas");
  reset_cmd->set_help_flag("-h,--help", "Show help for reset command");
  struct options {
    std::vector<std::string> names;
  };
  auto opts = std::make_shared<options>();
  reset_cmd->add_option("names", opts->names, "Formula names")->required(true);
  reset_cmd->callback([opts] {
    auto reset_res = ps_cli_endpoint_flux_formula_reset(opts->names);
    if (!ps_is_ok(reset_res)) {
      ps_exit_error(reset_res);
    }
  });
}

void register_formula_commands(CLI::App *app) {
  auto *formula_cmd = app->add_subcommand("formula", "Manage formulas");
  formula_cmd->require_subcommand(true);
  formula_cmd->set_help_flag("-h,--help", "Show help for formula commands");
  register_get_command(formula_cmd);
  register_list_command(formula_cmd);
  register_update_command(formula_cmd);
  register_reset_command(formula_cmd);
}
