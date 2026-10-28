#include "src/daemon/endpoint/flux/callback/job.hpp"
#include "croncpp.h"
#include "src/database/sqlite/formula.hpp"
#include "src/database/sqlite/info.hpp"
#include "src/database/sqlite/job.hpp"
#include "src/database/sqlite/timewin.hpp"
#include "src/lib/error.hpp"
#include "src/lib/formula.hpp"
#include "src/lib/job.hpp"
#include "src/lib/types.hpp"
#include <climits>
#include <cmath>
#include <flux/core.h>
#include <flux/core/job.h>
#include <jansson.h>
#include <regex>
#include <seanerbus/client/simple_event_loop.hpp>
#include <seanerbus/helpers/resolve.hpp>
#include <seanerbus/msg/message.hpp>
#include <set>
#include <src/daemon/endpoint/error.hpp>
#include <string>

struct ps_flux_job_t {
  flux_jobid_t id;
  int userid;
  int urgency;
  int priority;
  double t_submit;
  double t_depend;
  double t_run;
  double t_cleanup;
  double t_inactive;
  int state;
  std::string name;
  std::string cwd;
  int ntasks;
  int ncores;
  double duration;
  int nnodes;
  std::string ranks;
  std::string nodelist;
  bool success;
  bool exception_occured;
  // std::string exception_type;
  // int exception_severity;
  // std::string exception_note;
  int result;
  double expiration;
  // json_t annotations;
  // int waitstatus;
};

static ps_result_t<ps_success_t> ps_flux_job_ok(json_t *job_json) {
  flux_jobid_t id;
  double t_inactive = 0;
  bool exception_occured = false;
  if (json_unpack(job_json, "{s:I s?f s?b}", "id", &id, "t_inactive",
                  &t_inactive, "exception_occurred", &exception_occured) < 0) {
    return ps_error(
        ps_unexpected_err_t{.msg = "Failed to unpack job: " +
                                   std::string(json_dumps(job_json, 0))});
  }
  if (t_inactive != 0 && !exception_occured) {
    return ps_success_t{};
  }
  return ps_error(ps_job_invalid_t{.jobid = std::to_string(id)});
}

static ps_result_t<ps_flux_job_t> ps_flux_job_unpack(json_t *job_json) {
  ps_flux_job_t job;
  const char *name, *cwd, *ranks, *nodelist;
  if (json_unpack(job_json,
                  "{s:I s:i s:i s:i s:f s:f s:f s:f s:f s:i s:s s:s s:i s:i "
                  "s:f s:i s:s s:s s:b s:b s:i s:f}",
                  "id", &job.id, "userid", &job.userid, "urgency", &job.urgency,
                  "priority", &job.priority, "t_submit", &job.t_submit,
                  "t_depend", &job.t_depend, "t_run", &job.t_run, "t_cleanup",
                  &job.t_cleanup, "t_inactive", &job.t_inactive, "state",
                  &job.state, "name", &name, "cwd", &cwd, "ntasks", &job.ntasks,
                  "ncores", &job.ncores, "duration", &job.duration, "nnodes",
                  &job.nnodes, "ranks", &ranks, "nodelist", &nodelist,
                  "success", &job.success, "exception_occurred",
                  &job.exception_occured, "result", &job.result, "expiration",
                  &job.expiration) < 0) {
    return ps_error(
        ps_unexpected_err_t{.msg = "Failed to unpack job: " +
                                   std::string(json_dumps(job_json, 0))});
  }
  job.name = name;
  job.cwd = cwd;
  job.ranks = ranks;
  job.nodelist = nodelist;
  return job;
}

struct ps_flux_list_inactive_jobs_t {
  double t_inactive;
  std::vector<ps_flux_job_t> jobs;
};
static ps_result_t<ps_flux_list_inactive_jobs_t>
ps_flux_list_inactive_jobs(double since) {
  flux_t *h;
  if (!(h = flux_open(NULL, 0))) {
    return ps_error(ps_flux_open_err_t{});
  }

  flux_future_t *f;
  json_t *c;
  int version = 0;
  if (!(c = json_pack("{s:[i]}", "states", FLUX_JOB_STATE_INACTIVE))) {
    return ps_error(
        ps_unexpected_err_t{.msg = "Failed to pack job-list.list constraints"});
  }

  if (!(f = flux_rpc_pack(h, "job-list.list", FLUX_NODEID_ANY,
                          FLUX_RPC_STREAMING, "{s:i s:f s:[s] s:o}",
                          "max_entries", INT_MAX, "since", since, "attrs",
                          "all", "constraint", c))) {
    return ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
  }

  ps_flux_list_inactive_jobs_t res = {.t_inactive = since};
  while (1) {
    json_t *jobs;
    if (flux_rpc_get_unpack(f, "{s:o s?i}", "jobs", &jobs, "version",
                            &version) < 0) {
      if (errno == ENODATA) {
        break;
      }
      return ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
    }

    if (!json_is_array(jobs)) {
      return ps_error(ps_field_type_mismatch_t{.field = "job-list.list.jobs",
                                               .expected = "array"});
    }

    size_t index;
    json_t *value;
    json_array_foreach(jobs, index, value) {
      if (!ps_is_ok(ps_flux_job_ok(value))) {
        continue;
      }

      auto job_res = ps_flux_job_unpack(value);
      if (!ps_is_ok(job_res)) {
        return ps_forward_error<ps_flux_list_inactive_jobs_t>(job_res);
      }
      auto job = ps_unwrap(job_res);
      res.jobs.push_back(job);
      res.t_inactive = std::max(res.t_inactive, job.t_inactive);
    }

    /* version 0 protocol does not stream, all results in one response */
    if (version == 0) {
      break;
    }
    flux_future_reset(f);
  }

  flux_future_destroy(f);
  return res;
}

static ps_result_t<std::vector<ps_flux_job_t>>
ps_flux_list_jobs(const std::vector<flux_jobid_t> &jobids) {
  flux_future_t *f;
  json_t *c;
  int version = 0;
  flux_t *h;
  if (!(h = flux_open(NULL, 0))) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }

  std::vector<ps_flux_job_t> res;
  for (const auto &jobid : jobids) {
    flux_future_t *f;
    if (!(f = flux_job_list_id(h, jobid, "[\"all\"]"))) {
      return ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
    }
    json_t *value;
    if (flux_rpc_get_unpack(f, "{s:o}", "job", &value) < 0) {
      flux_future_destroy(f);
      return ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
    }

    auto ok_res = ps_flux_job_ok(value);
    if (!ps_is_ok(ok_res)) {
      return ps_forward_error<std::vector<ps_flux_job_t>>(ok_res);
    }

    auto job_res = ps_flux_job_unpack(value);
    if (!ps_is_ok(job_res)) {
      return ps_forward_error<std::vector<ps_flux_job_t>>(job_res);
    }
    auto job = ps_unwrap(job_res);
    res.push_back(job);
    flux_future_destroy(f);
  }
  return res;
}

static std::map<std::string, seanerbus::msg::Entity>
ps_formula_sb_entities(const ps_formula_t &formula,
                       const std::string &seanerbus_url) {
  std::map<std::string, seanerbus::msg::Entity> res;

  std::regex rpc_pattern(R"(<rpc:[^>]*>)");
  std::regex rpc_ns_pattern(R"(<rpc_ns:([^>]*)>)");
  std::regex adapter_pattern(R"(<adapter:[^>]*>)");

  auto collect = [&formula](const std::regex &pattern) {
    std::set<std::string> matches;
    for (auto it = std::sregex_iterator(formula.expr.begin(),
                                        formula.expr.end(), pattern);
         it != std::sregex_iterator(); ++it) {
      matches.insert((*it)[0].str());
    }
    return matches;
  };

  std::set<std::string> rpc_matches = collect(rpc_pattern);
  std::set<std::string> rpc_ns_matches = collect(rpc_ns_pattern);
  std::set<std::string> adapter_matches = collect(adapter_pattern);

  if (rpc_matches.empty() && rpc_ns_matches.empty() &&
      adapter_matches.empty()) {
    return res;
  }

  // <rpc:UUID>: the inner text is the entity UUID, used directly.
  for (const std::string &match : rpc_matches) {
    const std::string rpc_service_full =
        match.substr(1, match.length() - 2); // rpc:UUID
    const std::string rpc_service =
        rpc_service_full.substr(4, rpc_service_full.length() - 4); // UUID
    try {
      auto entity = seanerbus::msg::Entity(seanerbus::msg::Uuid(rpc_service));
      res[rpc_service_full] = entity;
    } catch (const std::invalid_argument &) {
      continue;
    }
  }

  // <rpc_ns:{...}>: the inner text is a JSON object (a map<string,string>)
  // naming the entity to resolve via the naming service, e.g.
  // <rpc_ns:{"service":"time_series_db"}>.
  for (const std::string &match : rpc_ns_matches) {
    const std::string rpc_ns_service_full =
        match.substr(1, match.length() - 2); // rpc_ns:{...}

    std::smatch json_m;
    if (!std::regex_search(match, json_m, rpc_ns_pattern) ||
        json_m.size() < 2) {
      continue;
    }
    const std::string rpc_ns_json = json_m[1].str();

    json_error_t json_err;
    json_t *json_obj = json_loads(rpc_ns_json.c_str(), 0, &json_err);
    if (!json_obj || !json_is_object(json_obj)) {
      json_decref(json_obj);
      continue;
    }

    std::map<std::string, std::string> resolve_name;
    const char *key = nullptr;
    json_t *value_j = nullptr;
    json_object_foreach(json_obj, key, value_j) {
      if (json_is_string(value_j)) {
        resolve_name[key] = json_string_value(value_j);
      }
    }
    json_decref(json_obj);

    seanerbus::msg::Entity entity;
    bool resolved = false;
    seanerbus::client::Connection conn(seanerbus_url, seanerbus_url);
    seanerbus::resolve::resolve(
        conn, resolve_name,
        [&](seanerbus::client::Connection &c,
            seanerbus::Expected<seanerbus::resolve::ResolveResult,
                                seanerbus::Error>
                res) {
          if (!res.ok()) {
            c.shutdown(seanerbus::Unexpected(res.unwrap_error()));
            return;
          }
          auto result = res.unwrap_ok();
          auto entities = result.entities();
          if (!entities.empty()) {
            entity = entities[0];
            resolved = true;
          }
          c.shutdown();
        });
    seanerbus::client::SimpleEventLoop sel(conn);
    sel.run();
    if (resolved) {
      res[rpc_ns_service_full] = entity;
    }
  }

  /* Unsupported for now
  for (const std::string &match : adapter_matches) {
    std::string inner = match.substr(2, match.length() - 4);
  }
  */
  return res;
}

static ps_result_t<double> ps_job_score_compute(
    ps_flux_job_t job, const ps_formula_t &formula,
    const std::vector<ps_timewin_t> &timewins, const std::string &seanerbus_url,
    std::map<std::string, seanerbus::msg::Entity> sb_entities) {
  std::map<std::string, double> vars_over;

  auto timewin_score = 0;
  for (const auto &timewin : timewins) {
    auto cron_expr = cron::make_cron(timewin.cron);
    auto prev_timestamp = cron::cron_prev(cron_expr, job.t_submit);
    if (prev_timestamp + timewin.duration > job.t_submit) {
      timewin_score += timewin.score;
    }
  }
  vars_over["var:timewin_score"] = timewin_score;

  auto real_duration = job.t_inactive - job.t_run;
  auto duration_ratio = real_duration / job.duration;
  if (!std::isinf(duration_ratio)) {
    vars_over["var:duration_ratio"] = std::min(1.0, duration_ratio);
  }

  if (sb_entities.size() > 0) {
    for (const auto &[var, entity] : sb_entities) {
      seanerbus::client::Connection req_conn(seanerbus_url, seanerbus_url);
      try {
        seanerbus::msg::MessageBuilder<HelloWorldReqV1> bd;
        bd.builder().setMsg(std::to_string(job.id));
        req_conn.request(
            bd.to_message(entity),
            [&,
             var](seanerbus::client::Connection &c,
                  seanerbus::Expected<seanerbus::msg::Message, seanerbus::Error>
                      res) {
              // The variable resolution silently fails
              // A warning could be emitted in this function
              if (!res.ok()) {
                return c.shutdown(seanerbus::Unexpected(res.unwrap_error()));
              }
              auto msg = std::move(res.unwrap_ok());
              if (msg.payload_type() != HelloWorldResV1::TYPE) {
                return c.shutdown();
              }
              auto rd_res =
                  seanerbus::msg::MessageReader<HelloWorldResV1>::from_msg(msg);
              if (!rd_res.ok()) {
                return c.shutdown();
              }
              auto rd = std::move(rd_res.unwrap_ok());
              const std::string res_str = rd.reader().getMsg().cStr();
              double res_double;
              if (!parse_double(res_str, res_double)) {
                return c.shutdown();
              }
              vars_over[var] = res_double;
              c.shutdown();
            });
        seanerbus::client::SimpleEventLoop req_sel(req_conn);
        req_sel.run();
      } catch (const seanerbus::Expected<seanerbus::msg::Message,
                                         seanerbus::Error>) {
        // The variable resolution silently fails
        // A warning could be emitted here
        continue;
      }
    }
  }

  auto eval_res = ps_eval_formula(formula, vars_over);
  if (!ps_is_ok(eval_res)) {
    return eval_res;
  }

  return ps_unwrap(eval_res);
}

static ps_result_t<std::string>
ps_flux_jobid_format(const std::string &jobid_str, const std::string &format) {
  flux_jobid_t id;
  if (flux_job_id_parse(jobid_str.c_str(), &id) != 0) {
    return ps_error(ps_flux_jobid_parse_err_t{.jobid = jobid_str,
                                              .strerror = strerror(errno)});
  }
  char buf[256];
  if (flux_job_id_encode(id, format.c_str(), buf, sizeof(buf)) != 0) {
    return ps_error(ps_unexpected_err_t{.msg = "Failed to format job id '" +
                                               jobid_str + "' as '" + format +
                                               "': " + strerror(errno)});
  }
  return std::string(buf);
}

int ps_daemon_endpoint_flux_job_get(
    const ps_daemon_endpoint_flux_callback_args_t &args) {
  // Retrieve the job id and the requested id format from the payload.
  const char *jobid_str;
  const char *format;
  if (flux_request_unpack(args.msg, NULL, "{s:s s:s}", "data", &jobid_str,
                          "format", &format) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }

  // Parse the job id
  flux_jobid_t jobid;
  if (flux_job_id_parse(jobid_str, &jobid) < 0) {
    auto err = ps_flux_jobid_parse_err_t{.jobid = jobid_str,
                                         .strerror = strerror(errno)};
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }

  // Retrieve the job from the database
  auto get_res = ps_db_sqlite_job_get(args.db, jobid);
  if (!ps_is_ok(get_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(get_res).c_str());
  }
  auto job = ps_unwrap(get_res);

  // Authorize: owners may read any job, others only their own
  uint32_t userid;
  uint32_t rolemask;
  if (flux_msg_get_userid(args.msg, &userid) < 0 ||
      flux_msg_get_rolemask(args.msg, &rolemask) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }
  if (!(rolemask & FLUX_ROLE_OWNER) &&
      job.userid != static_cast<uid_t>(userid)) {
    auto err = ps_access_denied_t{.key = "job." + std::string(jobid_str)};
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }

  // Render the id in the requested format before returning.
  auto fmt_res = ps_flux_jobid_format(job.jobid, format);
  if (!ps_is_ok(fmt_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(fmt_res).c_str());
  }
  job.jobid = ps_unwrap(fmt_res);

  return flux_respond_pack(args.h, args.msg, "{s:o}", "data",
                           ps_job_to_json(job));
}

int ps_daemon_endpoint_flux_job_list(
    const ps_daemon_endpoint_flux_callback_args_t &args) {
  // Identify the caller: owners may list every job, others only their own
  uint32_t userid;
  uint32_t rolemask;
  if (flux_msg_get_userid(args.msg, &userid) < 0 ||
      flux_msg_get_rolemask(args.msg, &rolemask) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }
  auto is_owner = (rolemask & FLUX_ROLE_OWNER) != 0;

  // Retrieve the array of job ids (optional: empty means list all), the
  // requested id format, and the row limit from the payload.
  json_t *data_json = nullptr;
  const char *format;
  int limit;
  if (flux_request_unpack(args.msg, NULL, "{s?o s:s s:i}", "data", &data_json,
                          "format", &format, "limit", &limit) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }

  std::vector<ps_job_t> jobs;
  if (data_json == nullptr) {
    // No parameter: return the authorized list, capped at `limit` rows
    // (limit <= 0 means no cap).
    size_t limit_entries = limit > 0 ? static_cast<size_t>(limit) : SIZE_MAX;
    ps_result_t<std::vector<ps_job_t>> list_res;
    if (is_owner) {
      list_res = ps_db_sqlite_jobs_list(args.db, limit_entries);
    } else {
      // Non-owners only see their own jobs
      list_res = ps_db_sqlite_jobs_list(args.db, limit_entries, userid);
    }
    if (!ps_is_ok(list_res)) {
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(list_res).c_str());
    }
    jobs = ps_unwrap(list_res);
  } else {
    if (!json_is_array(data_json)) {
      auto err = ps_field_type_mismatch_t{.field = "data", .expected = "array"};
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(err).c_str());
    }

    // Build the list of job ids
    std::vector<flux_jobid_t> jobids;
    size_t index;
    json_t *value;
    json_array_foreach(data_json, index, value) {
      if (!json_is_string(value)) {
        auto err = ps_field_type_mismatch_t{
            .field = "data[" + std::to_string(index) + "]",
            .expected = "string"};
        return flux_respond_error(args.h, args.msg, 0,
                                  ps_error_message(err).c_str());
      }
      std::string jobid_str = json_string_value(value);
      flux_jobid_t jobid;
      if (flux_job_id_parse(jobid_str.c_str(), &jobid) < 0) {
        auto err = ps_flux_jobid_parse_err_t{.jobid = jobid_str,
                                             .strerror = strerror(errno)};
        return flux_respond_error(args.h, args.msg, 0,
                                  ps_error_message(err).c_str());
      }
      jobids.push_back(jobid);
    }

    // Retrieve the jobs from the database
    auto get_res = ps_db_sqlite_jobs_get(args.db, jobids);
    if (!ps_is_ok(get_res)) {
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(get_res).c_str());
    }
    auto jobs_map = ps_unwrap(get_res);

    // Build the list of jobs, preserving the request order
    for (const auto &jobid : jobids) {
      // The job is guaranteed to be in the map
      auto &job_res = jobs_map[jobid];
      if (!ps_is_ok(job_res)) {
        return flux_respond_error(args.h, args.msg, 0,
                                  ps_error_message(job_res).c_str());
      }
      auto job = ps_unwrap(job_res);
      // Non-owners can only see their own jobs
      if (!is_owner && job.userid != static_cast<uid_t>(userid)) {
        auto err = ps_access_denied_t{.key = "job." + job.jobid};
        return flux_respond_error(args.h, args.msg, 0,
                                  ps_error_message(err).c_str());
      }
      jobs.push_back(job);
    }
  }

  // Render every job id in the requested format before returning
  for (auto &job : jobs) {
    auto fmt_res = ps_flux_jobid_format(job.jobid, format);
    if (!ps_is_ok(fmt_res)) {
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(fmt_res).c_str());
    }
    job.jobid = ps_unwrap(fmt_res);
  }

  // Return as an array of jobs
  json_t *jobs_json = json_array();
  for (const auto &job : jobs) {
    json_array_append(jobs_json, ps_job_to_json(job));
  }
  return flux_respond_pack(args.h, args.msg, "{s:o}", "data", jobs_json);
}

int ps_daemon_endpoint_flux_job_update(
    const ps_daemon_endpoint_flux_callback_args_t &args) {
  bool _new;
  json_t *jobids_json = nullptr;
  if (flux_request_unpack(args.msg, NULL, "{s:{s:b s?o}}", "data", "new", &_new,
                          "jobids", &jobids_json) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }

  std::map<flux_jobid_t, ps_flux_job_t> jobs;
  double t_inactive;

  if (jobids_json != nullptr) {
    // Retrieve the list of jobids from the array
    if (!json_is_array(jobids_json)) {
      auto err =
          ps_field_type_mismatch_t{.field = "jobids", .expected = "array"};
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(err).c_str());
    }

    std::vector<flux_jobid_t> jobids;
    size_t index;
    json_t *value;
    json_array_foreach(jobids_json, index, value) {
      if (!json_is_string(value)) {
        auto err = ps_field_type_mismatch_t{
            .field = "jobids[" + std::to_string(index) + "]",
            .expected = "string"};
        return flux_respond_error(args.h, args.msg, 0,
                                  ps_error_message(err).c_str());
      }

      std::string jobid_str = json_string_value(value);
      flux_jobid_t jobid;
      if (flux_job_id_parse(jobid_str.c_str(), &jobid) < 0) {
        auto err = ps_flux_jobid_parse_err_t{.jobid = jobid_str,
                                             .strerror = strerror(errno)};
        return flux_respond_error(args.h, args.msg, 0,
                                  ps_error_message(err).c_str());
      }
      jobids.push_back(jobid);
    }

    auto jobs_res = ps_flux_list_jobs(jobids);
    if (!ps_is_ok(jobs_res)) {
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(jobs_res).c_str());
    }
    auto _jobs = ps_unwrap(jobs_res);
    for (const auto &job : _jobs) {
      jobs[job.id] = job;
    }
  }

  if (_new) {
    // Retrieve the new inactive jobs
    auto info_res = ps_db_sqlite_info_get(args.db);
    if (!ps_is_ok(info_res)) {
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(info_res).c_str());
    }
    // `since` is the current cursor; the scan returns the new (advanced) one.
    auto since = ps_unwrap(info_res).t_inactive;
    auto jobs_res = ps_flux_list_inactive_jobs(since);
    if (!ps_is_ok(jobs_res)) {
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(jobs_res).c_str());
    }
    auto _jobs = ps_unwrap(jobs_res);

    for (const auto &job : _jobs.jobs) {
      jobs[job.id] = job;
    }
    t_inactive = _jobs.t_inactive;
  }

  // Retrieve the list of timewins
  auto timewins_res = ps_db_sqlite_timewins_list(args.db);
  if (!ps_is_ok(timewins_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(timewins_res).c_str());
  }
  auto timewins = ps_unwrap(timewins_res);

  // Retrieve the formula
  auto formula_res = ps_db_sqlite_formula_get(args.db, "job_score");
  if (!ps_is_ok(formula_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(formula_res).c_str());
  }
  auto formula = ps_unwrap(formula_res);

  // Retrieve the seanerbus entities in the formula
  auto sb_entities = ps_formula_sb_entities(formula, args.seanerbus_url);

  std::vector<ps_job_t> _jobs;
  for (const auto &[jobid, job] : jobs) {
    auto score_res = ps_job_score_compute(job, formula, timewins,
                                          args.seanerbus_url, sb_entities);
    if (!ps_is_ok(score_res)) {
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(score_res).c_str());
    }
    auto uid = static_cast<uid_t>(job.userid);
    _jobs.push_back({.jobid = std::to_string(jobid),
                     .userid = uid,
                     .t_inactive = job.t_inactive,
                     .score = ps_unwrap(score_res),
                     .duration = job.t_inactive - job.t_run});
  }

  // Store the scored jobs and, when scanning newly inactive jobs, advance the
  // t_inactive cursor in a single transaction. Both writes commit or roll back
  // together.
  ps_result_t<ps_success_t> set_res =
      _new ? ps_db_sqlite_jobs_set(args.db, _jobs, t_inactive)
           : ps_db_sqlite_jobs_set(args.db, _jobs);
  if (!ps_is_ok(set_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(set_res).c_str());
  }

  return flux_respond(args.h, args.msg, NULL);
}
