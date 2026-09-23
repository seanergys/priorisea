// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "flux.hpp"
#include "error.hpp"
#include "formula.hpp"
#include "types.hpp"
#include "util.hpp"
#include <flux/core.h>
#include <flux/core/future.h>
#include <flux/core/job.h>
#include <flux/core/rpc.h>
#include <flux/core/types.h>
#include <jansson.h>
#include <sys/types.h>

ps_result_t<ps_formula_t> ps_flux_plugin_get_formula() {
  flux_t *h;
  flux_future_t *f;
  if (!(h = flux_open(NULL, 0))) {
    return ps_error(ps_flux_open_err_t{.msg = strerror(errno)});
  }
  if (!(f = flux_rpc(h, "job-manager.sea_priority.get_formula", "",
                     FLUX_NODEID_ANY, 0))) {
    flux_close(h);
    return ps_error(ps_flux_rpc_err_t{.msg = strerror(errno)});
  }
  json_t *json_formula = nullptr;
  if (flux_rpc_get_unpack(f, "{s:o}", "data", &json_formula) < 0) {
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(ps_flux_rpc_get_unpack_err_t{.msg = strerror(errno)},
                    "unpacking rpc job-manager.sea_priority.get_formula. Is "
                    "the plugin loaded?");
  }

  auto formula = ps_formula_from_json(json_formula);
  json_decref(json_formula);
  flux_future_destroy(f);
  flux_close(h);
  return formula;
}

ps_result_t<ps_success_t>
ps_flux_plugin_set_formula(const ps_formula_t &formula) {
  flux_t *h;
  flux_future_t *f;
  if (!(h = flux_open(NULL, 0))) {
    return ps_error(ps_flux_open_err_t{.msg = strerror(errno)});
  }
  json_t *json_formula = ps_formula_to_json(formula);
  if (!(f = flux_rpc_pack(h, "job-manager.sea_priority.set_formula",
                          FLUX_NODEID_ANY, 0, "{s:o}", "data", json_formula))) {
    flux_close(h);
    return ps_error(ps_flux_rpc_pack_err_t{.msg = strerror(errno)});
  }
  if (flux_future_get(f, NULL) < 0) {
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(ps_flux_future_get_err_t{.msg = strerror(errno)},
                    "getting rpc job-manager.sea_priority.set_formula. Is the "
                    "plugin loaded?");
  }
  flux_future_destroy(f);
  flux_close(h);
  return ps_success_t{};
}

ps_result_t<ps_score_map_t>
ps_flux_plugin_get_score(const std::vector<int> &userids) {
  flux_t *h;
  flux_future_t *f;
  if (!(h = flux_open(NULL, 0))) {
    return ps_error(ps_flux_open_err_t{.msg = strerror(errno)});
  }

  json_t *data = json_array();
  for (int userid : userids) {
    json_array_append_new(data, json_integer(userid));
  }
  if (!(f = flux_rpc_pack(h, "job-manager.sea_priority.get_score",
                          FLUX_NODEID_ANY, 0, "{s:o}", "data", &data))) {
    json_decref(data);
    flux_close(h);
    return ps_error(ps_flux_rpc_pack_err_t{.msg = strerror(errno)});
  }

  json_t *scores = nullptr;
  if (flux_rpc_get_unpack(f, "{s:o}", "data", &scores) < 0) {
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(ps_flux_rpc_get_unpack_err_t{.msg = strerror(errno)},
                    "unpacking rpc job-manager.sea_priority.get_score. Is the "
                    "plugin loaded?");
  }

  auto score_map = score_map_from_json(scores);
  json_decref(scores);
  flux_future_destroy(f);
  flux_close(h);
  return score_map;
}

ps_result_t<ps_success_t>
ps_flux_plugin_set_score(const ps_score_map_t &scores) {
  flux_t *h;
  flux_future_t *f;
  if (!(h = flux_open(NULL, 0))) {
    return ps_error(ps_flux_open_err_t{.msg = strerror(errno)});
  }
  json_t *scores_json = score_map_to_json(scores);
  if (!(f = flux_rpc_pack(h, "job-manager.sea_priority.set_score",
                          FLUX_NODEID_ANY, 0, "{s:o}", "data", scores_json))) {
    json_decref(scores_json);
    flux_close(h);
    return ps_error(ps_flux_rpc_pack_err_t{.msg = strerror(errno)});
  }
  if (flux_future_get(f, NULL) < 0) {
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(ps_flux_future_get_err_t{.msg = strerror(errno)},
                    "getting rpc job-manager.sea_priority.set_score. Is the "
                    "plugin loaded?");
  }
  flux_future_destroy(f);
  flux_close(h);
  return ps_success_t{};
}

ps_result_t<ps_success_t> ps_flux_kvs_commit(flux_kvs_txn_t *txn) {
  flux_t *h;
  flux_future_t *f;
  if (!(h = flux_open(NULL, 0))) {
    return ps_error(ps_flux_open_err_t{.msg = strerror(errno)});
  }
  if (!(f = flux_kvs_commit(h, NULL, 0, txn))) {
    flux_close(h);
    return ps_error(ps_flux_kvs_commit_err_t{.msg = strerror(errno)});
  }

  if (flux_future_get(f, NULL) < 0) {
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(ps_flux_future_get_err_t{.msg = strerror(errno)});
  }
  flux_future_destroy(f);
  flux_close(h);
  return ps_success_t{};
}

ps_result_t<std::vector<ps_flux_kvs_remove_entry_t>>
ps_flux_kvs_remove(const std::vector<std::string> &keys) {
  flux_t *h;
  if (!(h = flux_open(NULL, 0))) {
    return ps_error(ps_flux_open_err_t{.msg = strerror(errno)});
  }
  std::vector<ps_flux_kvs_remove_entry_t> res;
  for (auto key : keys) {
    flux_kvs_txn_t *t;
    if (!(t = flux_kvs_txn_create())) {
      res.push_back({.key = key,
                     .result = ps_error(ps_flux_kvs_txn_create_err_t{
                         .msg = strerror(errno)})});
      continue;
    }
    if (flux_kvs_txn_unlink(t, 0, key.c_str()) < 0) {
      flux_kvs_txn_destroy(t);
      res.push_back({.key = key,
                     .result = ps_error(ps_flux_kvs_txn_unlink_err_t{
                         .key = key, .msg = strerror(errno)})});
      continue;
    }
    flux_future_t *f;
    if (!(f = flux_kvs_commit(h, NULL, 0, t))) {
      flux_kvs_txn_destroy(t);
      res.push_back({.key = key,
                     .result = ps_error(
                         ps_flux_kvs_commit_err_t{.msg = strerror(errno)})});
      continue;
    }
    if (flux_future_get(f, NULL) < 0) {
      res.push_back({.key = key,
                     .result = ps_error(
                         ps_flux_future_get_err_t{.msg = strerror(errno)})});
      flux_future_destroy(f);
      continue;
    }
    res.push_back({.key = key, .result = ps_success_t{}});
    flux_future_destroy(f);
  }
  flux_close(h);
  return res;
}

ps_result_t<ps_success_t> ps_flux_kvs_remove(const std::string &key) {
  auto remove_res = ps_flux_kvs_remove(std::vector<std::string>{key});
  if (!ps_is_ok(remove_res)) {
    return ps_forward_error<ps_success_t>(remove_res, "removing key '" + key +
                                                          "' from flux kvs");
  }
  return ps_unwrap(remove_res)[0].result;
}

ps_result_t<std::vector<ps_flux_kvs_lookup_entry_t>>
ps_flux_kvs_lookup(const std::vector<std::string> &keys) {
  flux_t *h;
  if (!(h = flux_open(NULL, 0))) {
    return ps_error(ps_flux_open_err_t{.msg = strerror(errno)});
  }

  std::vector<ps_flux_kvs_lookup_entry_t> res;
  for (const auto &key : keys) {
    flux_future_t *f;
    if (!(f = flux_kvs_lookup(h, NULL, 0, key.c_str()))) {
      res.push_back({.key = key,
                     .value = ps_error(ps_flux_kvs_lookup_err_t{.key = key})});
      continue;
    }

    const char *value;
    if (flux_kvs_lookup_get(f, &value) < 0) {
      flux_future_destroy(f);
      res.push_back(
          {.key = key,
           .value = ps_error(ps_flux_kvs_lookup_get_err_t{.key = key})});
      continue;
    }

    auto value_str = std::string(value);
    if (value_str == "") {
      flux_future_destroy(f);
      res.push_back(
          {.key = key,
           .value = ps_error(ps_flux_kvs_lookup_get_err_t{.key = key})});
      continue;
    }
    res.push_back({.key = key, .value = value_str});
    flux_future_destroy(f);
  }

  flux_close(h);
  return res;
}

ps_result_t<std::string> ps_flux_kvs_lookup(const std::string &key) {
  auto res = ps_flux_kvs_lookup(std::vector<std::string>{key});
  if (ps_is_ok(res)) {
    auto _res = ps_unwrap(res);
    return _res[0].value;
  }
  return ps_forward_error<std::string>(res, "looking up key '" + key +
                                                "' in flux kvs");
}

ps_result_t<ps_flux_job_info_t> ps_flux_job_info_unpack(json_t *value) {
  ps_flux_job_info_t res;
  const char *name, *cwd, *ranks, *nodelist;
  if (json_unpack(value,
                  "{s:I s:i s:i s:i s:f s:f s:f s:f s:f s:i s:s s:s s:i s:i "
                  "s:f s:i s:s s:s s:b s:b s:i s:f}",
                  "id", &res.id, "userid", &res.userid, "urgency", &res.urgency,
                  "priority", &res.priority, "t_submit", &res.t_submit,
                  "t_depend", &res.t_depend, "t_run", &res.t_run, "t_cleanup",
                  &res.t_cleanup, "t_inactive", &res.t_inactive, "state",
                  &res.state, "name", &name, "cwd", &cwd, "ntasks", &res.ntasks,
                  "ncores", &res.ncores, "duration", &res.duration, "nnodes",
                  &res.nnodes, "ranks", &ranks, "nodelist", &nodelist,
                  "success", &res.success, "exception_occurred",
                  &res.exception_occured, "result", &res.result, "expiration",
                  &res.expiration) < 0) {
    return ps_error(ps_json_unpack_err_t{.dumps = json_dumps(value, 0)});
  }
  res.name = name;
  res.cwd = cwd;
  res.ranks = ranks;
  res.nodelist = nodelist;
  return res;
}

ps_result_t<ps_success_t> ps_flux_list_inactive_jobs_stream(
    double since, int max_entries,
    std::function<ps_result_t<ps_success_t>(json_t *)> callback) {
  flux_t *h;
  if (!(h = flux_open(NULL, 0))) {
    return ps_error(ps_flux_open_err_t{});
  }

  flux_future_t *f;
  json_t *c;
  int version = 0;
  if (!(c = json_pack("{s:[i]}", "states", FLUX_JOB_STATE_INACTIVE))) {
    return ps_error(ps_json_pack_err_t{});
  }

  if (!(f = flux_rpc_pack(h, "job-list.list", FLUX_NODEID_ANY,
                          FLUX_RPC_STREAMING, "{s:i s:f s:[s] s:o}",
                          "max_entries", max_entries, "since", since, "attrs",
                          "all", "constraint", c))) {
    return ps_error(ps_flux_rpc_pack_err_t{.msg = strerror(errno)});
  }

  double t_inactive_max = since;
  while (1) {
    json_t *jobs;
    if (flux_rpc_get_unpack(f, "{s:o s?i}", "jobs", &jobs, "version",
                            &version) < 0) {
      if (errno == ENODATA) {
        break;
      }
      return ps_error(ps_flux_rpc_get_unpack_err_t{.msg = strerror(errno)});
    }

    auto callback_res = callback(jobs);
    if (!ps_is_ok(callback_res)) {
      flux_future_destroy(f);
      return callback_res;
    }

    /* version 0 protocol does not stream, all results in one response */
    if (version == 0) {
      break;
    }
    flux_future_reset(f);
  }
  flux_future_destroy(f);
  return ps_success_t{};
}

ps_result_t<ps_flux_kvs_list_inactive_t>
ps_flux_list_inactive_jobs(double since, int max_entries) {
  double t_inactive_max = since;
  ps_flux_kvs_list_inactive_t res;
  auto stream_res = ps_flux_list_inactive_jobs_stream(
      since, max_entries, [&t_inactive_max, &res](json_t *jobs) {
        size_t index;
        json_t *value;
        json_array_foreach(jobs, index, value) {
          auto unpack_res = ps_flux_job_info_unpack(value);
          if (!ps_is_ok(unpack_res)) {
            return ps_forward_error<ps_success_t>(unpack_res,
                                                  "unpacking job info");
          }
          res.result.push_back(unpack_res);

          t_inactive_max =
              std::max(t_inactive_max, ps_unwrap(unpack_res).t_inactive);
        }
        return (ps_result_t<ps_success_t>)ps_success_t{};
      });
  if (!ps_is_ok(stream_res)) {
    return ps_forward_error<ps_flux_kvs_list_inactive_t>(
        stream_res, "listing inactive jobs");
  }
  res.t_inactive_max = t_inactive_max;
  return res;
}

ps_result_t<std::vector<ps_flux_job_entry_t>>
ps_flux_list_jobs(const std::vector<flux_jobid_t> &jobids) {
  flux_future_t *f;
  json_t *c;
  int version = 0;
  flux_t *h;
  if (!(h = flux_open(NULL, 0))) {
    return ps_error(ps_flux_open_err_t{.msg = strerror(errno)});
  }

  std::vector<ps_flux_job_entry_t> res;
  for (const auto &jobid : jobids) {
    flux_future_t *f;
    if (!(f = flux_job_list_id(h, jobid, "[\"all\"]"))) {
      res.push_back({.jobid = jobid,
                     .result = ps_error(ps_flux_job_list_id_err_t{
                         .jobid = jobid, .msg = strerror(errno)})});
      continue;
    }
    json_t *value;
    if (flux_rpc_get_unpack(f, "{s:o}", "job", &value) < 0) {
      res.push_back({.jobid = jobid,
                     .result = ps_error(ps_flux_rpc_get_unpack_err_t{
                         .msg = strerror(errno)})});
      flux_future_destroy(f);
      continue;
    }
    res.push_back({.jobid = jobid, .result = ps_flux_job_info_unpack(value)});
    flux_future_destroy(f);
  }
  return res;
}

ps_result_t<std::vector<std::string>>
ps_flux_kvs_ls_dir(const std::string &key) {
  flux_t *h;
  if (!(h = flux_open(NULL, 0))) {
    return ps_error(ps_flux_open_err_t{.msg = strerror(errno)});
  }
  flux_future_t *f;
  if (!(f = flux_kvs_lookup(h, NULL, FLUX_KVS_READDIR, key.c_str()))) {
    return ps_error(
        ps_flux_kvs_lookup_err_t{.key = key, .msg = strerror(errno)});
  }
  const flux_kvsdir_t *dir;
  if (flux_kvs_lookup_get_dir(f, &dir) < 0) {
    return ps_error(
        ps_flux_kvs_lookup_get_dir_err_t{.key = key, .msg = strerror(errno)});
  }

  flux_kvsitr_t *itr;

  if (!(itr = flux_kvsitr_create(dir))) {
    flux_future_destroy(f);
    return ps_error(
        ps_flux_kvsitr_create_err_t{.key = key, .msg = strerror(errno)});
  }

  std::vector<std::string> res;
  const char *name;
  while ((name = flux_kvsitr_next(itr))) {
    res.push_back(name);
  }
  return res;
}

ps_result_t<ps_success_t>
ps_flux_plugin_set_seanerbus(const ps_seanerbus_t &seanerbus) {
  flux_t *h;
  flux_future_t *f;
  if (!(h = flux_open(NULL, 0))) {
    return ps_error(ps_flux_open_err_t{.msg = strerror(errno)});
  }

  json_t *data = ps_seanerbus_to_json(seanerbus);
  if (!(f = flux_rpc_pack(h, "job-manager.sea_priority.set_seanerbus",
                          FLUX_NODEID_ANY, 0, "{s:o}", "data", data))) {
    flux_close(h);
    json_decref(data);
    return ps_error(ps_flux_rpc_pack_err_t{.msg = strerror(errno)});
  }

  if (flux_future_get(f, NULL) < 0) {
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(
        ps_flux_future_get_err_t{.msg = strerror(errno)},
        "getting rpc job-manager.sea_priority.set_seanerbus. Is the "
        "plugin loaded?");
  }
  flux_future_destroy(f);
  flux_close(h);
  return ps_success_t{};
}

ps_result_t<ps_seanerbus_t> ps_flux_plugin_get_seanerbus() {
  flux_t *h;
  flux_future_t *f;
  if (!(h = flux_open(NULL, 0))) {
    return ps_error(ps_flux_open_err_t{.msg = strerror(errno)});
  }

  if (!(f = flux_rpc(h, "job-manager.sea_priority.get_seanerbus", "",
                     FLUX_NODEID_ANY, 0))) {
    flux_close(h);
    return ps_error(ps_flux_rpc_err_t{.msg = strerror(errno)});
  }

  json_t *data = nullptr;
  if (flux_rpc_get_unpack(f, "{s:o}", "data", &data) < 0) {
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(ps_flux_rpc_get_unpack_err_t{.msg = strerror(errno)},
                    "unpacking rpc job-manager.sea_priority.get_seanerbus. Is "
                    "the plugin loaded?");
  }

  auto seanerbus = ps_seanerbus_from_json(data);
  json_decref(data);
  flux_future_destroy(f);
  flux_close(h);
  return seanerbus;
}

////

json_t *ps_seanerbus_to_json(const ps_seanerbus_t &seanerbus) {
  auto *obj = json_object();
  json_object_set_new(obj, "ip", json_string(seanerbus.ip.c_str()));
  json_object_set_new(obj, "port", json_integer(seanerbus.port));
  return obj;
}

ps_seanerbus_t ps_seanerbus_from_json(json_t *obj) {
  ps_seanerbus_t res;
  const char *ip = nullptr;
  int port = 0;
  json_unpack(obj, "{s:s, s:i}", "ip", &ip, "port", &port);
  res.ip = ip ? ip : "";
  res.port = port;
  return res;
}

json_t *ps_flux_job_info_to_json(const ps_flux_job_info_t &info) {
  auto *obj = json_object();
  json_object_set_new(obj, "id", json_integer(info.id));
  json_object_set_new(obj, "userid", json_integer(info.userid));
  json_object_set_new(obj, "urgency", json_integer(info.urgency));
  json_object_set_new(obj, "priority", json_integer(info.priority));
  json_object_set_new(obj, "t_submit", json_real(info.t_submit));
  json_object_set_new(obj, "t_depend", json_real(info.t_depend));
  json_object_set_new(obj, "t_run", json_real(info.t_run));
  json_object_set_new(obj, "t_cleanup", json_real(info.t_cleanup));
  json_object_set_new(obj, "t_inactive", json_real(info.t_inactive));
  json_object_set_new(obj, "state", json_integer(info.state));
  json_object_set_new(obj, "name", json_string(info.name.c_str()));
  json_object_set_new(obj, "cwd", json_string(info.cwd.c_str()));
  json_object_set_new(obj, "ntasks", json_integer(info.ntasks));
  json_object_set_new(obj, "ncores", json_integer(info.ncores));
  json_object_set_new(obj, "duration", json_real(info.duration));
  json_object_set_new(obj, "nnodes", json_integer(info.nnodes));
  json_object_set_new(obj, "ranks", json_string(info.ranks.c_str()));
  json_object_set_new(obj, "nodelist", json_string(info.nodelist.c_str()));
  json_object_set_new(obj, "success", json_boolean(info.success));
  json_object_set_new(obj, "exception_occurred",
                      json_boolean(info.exception_occured));
  json_object_set_new(obj, "result", json_integer(info.result));
  json_object_set_new(obj, "expiration", json_real(info.expiration));
  return obj;
}

json_t *score_map_to_json(const ps_score_map_t &scores) {
  auto *obj = json_object();
  for (const auto &[user_id, score] : scores) {
    json_object_set_new(obj, std::to_string(user_id).c_str(), json_real(score));
  }
  return obj;
}

ps_score_map_t score_map_from_json(json_t *obj) {
  ps_score_map_t score_map;
  const char *userid_key;
  json_t *score_value;
  json_object_foreach(obj, userid_key, score_value) {
    uid_t userid;
    if (!parse_uid(std::string(userid_key), userid)) {
      continue;
    }
    double score = json_real_value(score_value);
    score_map[userid] = score;
  }
  return score_map;
}
