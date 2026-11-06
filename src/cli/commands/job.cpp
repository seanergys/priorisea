#include "src/cli/commands/job.hpp"
#include "src/cli/endpoint/flux/job.hpp"
#include "src/cli/util.hpp"
#include "src/lib/error.hpp"
#include "src/lib/job.hpp"
#include "src/lib/util.hpp"
#include <cerrno>
#include <climits>
#include <flux/core.h>
#include <jansson.h>
#include <memory>
#include <string>
#include <sys/types.h>
#include <vector>

static void print_job(const ps_job_t &job, bool json, bool plain) {
  if (json) {
    json_t *obj;
    if (plain) {
      obj = ps_job_to_json_plain(job);
    } else {
      obj = ps_job_to_json(job);
    }
    char *dump = json_dumps(obj, JSON_INDENT(2));
    if (dump) {
      std::cout << dump << std::endl;
      free(dump);
    }
    json_decref(obj);
    return;
  }
  print_jobs_table(std::vector<ps_job_t>{job}, plain);
}

static void print_jobs(const std::vector<ps_job_t> &jobs, bool json,
                       bool plain) {
  if (json) {
    auto arr = json_array();
    for (const auto &job : jobs) {
      json_t *obj;
      if (plain) {
        obj = ps_job_to_json_plain(job);
      } else {
        obj = ps_job_to_json(job);
      }
      json_array_append_new(arr, obj);
    }
    char *dump = json_dumps(arr, JSON_INDENT(2));
    if (dump) {
      std::cout << dump << std::endl;
      free(dump);
    }
    json_decref(arr);
    return;
  }
  print_jobs_table(jobs, plain);
}

static void register_get_command(CLI::App *app) {
  auto *get_cmd = app->add_subcommand("get", "Get a job");
  get_cmd->set_help_flag("-h,--help", "Show help for get command");
  struct options {
    std::string jobid;
    std::string format;
    bool json;
    bool detail;
  };
  auto opts = std::make_shared<options>();
  get_cmd->add_option("jobid", opts->jobid, "Job ID")->required(true);
  get_cmd->add_flag("-d,--detail", opts->detail, "Show additional details");
  get_cmd->add_option("-f,--format", opts->format, "Job ID output format")
      ->default_val("f58")
      ->check(CLI::IsMember(
          {"dec", "hex", "kvs", "dothex", "words", "f58", "f58plain"}));
  get_cmd->add_flag("-j,--json", opts->json, "Output JSON");
  get_cmd->callback([opts] {
    auto get_res = ps_cli_endpoint_flux_job_get(opts->jobid, opts->format);
    if (!ps_is_ok(get_res)) {
      ps_exit_error(get_res);
    }
    print_job(ps_unwrap(get_res), opts->json, !opts->detail);
  });
}

static void register_list_command(CLI::App *app) {
  auto *list_cmd = app->add_subcommand("list", "List jobs");
  list_cmd->set_help_flag("-h,--help", "Show help for list command");
  struct options {
    std::vector<std::string> jobids;
    std::string format;
    int limit;
    bool json;
    bool detail;
    std::string order;
    std::string sort;
  };
  auto opts = std::make_shared<options>();
  list_cmd->add_option("jobids", opts->jobids, "Job IDs (optional)");
  list_cmd->add_flag("-d,--detail", opts->detail, "Show additional details");
  list_cmd->add_option("-f,--format", opts->format, "Job IDs output format")
      ->check(CLI::IsMember(
          {"dec", "hex", "kvs", "dothex", "words", "f58", "f58plain"}))
      ->default_val("f58");
  list_cmd->add_flag("-j,--json", opts->json, "Output JSON");
  list_cmd->add_option("-n,--limit", opts->limit, "Limit the output to N items")
      ->default_val(INT_MAX);
  list_cmd->add_option("-o,--order", opts->order, "Order of the output")
      ->check(CLI::IsMember({"asc", "desc"}))
      ->default_val("desc");
  list_cmd->add_option("-s,--sort", opts->sort, "Column to order by")
      ->check(CLI::IsMember({"userid", "t_inactive", "score", "duration"}))
      ->default_val("t_inactive");
  list_cmd->callback([opts] {
    auto order_res = ps_db_order_from_str(opts->order);
    if (!ps_is_ok(order_res)) {
      ps_exit_error(order_res);
    }
    auto order = ps_unwrap(order_res);
    auto sort_res = ps_db_sort_from_str(opts->sort);
    if (!ps_is_ok(sort_res)) {
      ps_exit_error(sort_res);
    }
    auto sort = ps_unwrap(sort_res);
    auto list_res = ps_cli_endpoint_flux_job_list(opts->jobids, opts->format,
                                                  opts->limit, order, sort);
    if (!ps_is_ok(list_res)) {
      ps_exit_error(list_res);
    }
    print_jobs(ps_unwrap(list_res), opts->json, !opts->detail);
  });
}

static void register_update_command(CLI::App *app) {
  auto *update_cmd = app->add_subcommand("update", "Update jobs");
  update_cmd->set_help_flag("-h,--help", "Show help for list command");
  struct options {
    std::vector<std::string> jobids;
    bool _new;
  };
  auto opts = std::make_shared<options>();
  update_cmd->add_option("jobids", opts->jobids, "Job IDs (optional)");
  update_cmd->add_flag("-n,--new", opts->_new, "Update new jobs");
  update_cmd->callback([opts] {
    auto update_res = ps_cli_endpoint_flux_job_update(opts->jobids, opts->_new);
    if (!ps_is_ok(update_res)) {
      ps_exit_error(update_res);
    }
  });
}

void register_job_commands(CLI::App *app) {
  auto *job_cmd = app->add_subcommand("job", "Manage jobs");
  job_cmd->require_subcommand(true);
  job_cmd->set_help_flag("-h,--help", "Show help for job commands");
  register_get_command(job_cmd);
  register_list_command(job_cmd);
  register_update_command(job_cmd);
}
