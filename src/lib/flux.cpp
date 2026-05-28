// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "flux.hpp"
#include "util.hpp"
#include <flux/core.h>
#include <flux/core/future.h>
#include <flux/core/rpc.h>
#include <jansson.h>

flux_plugin_get_formula_res flux_plugin_get_formula() {
  flux_t *h;
  flux_future_t *f;
  if (!(h = flux_open(NULL, 0))) {
    return {
        flux_plugin_res{
            .err = FLUX_PLUGIN_ERR_OPEN,
            .err_msg = strerror(errno),
        },
    };
  }
  if (!(f = flux_rpc(h, "job-manager.sea_priority.get_formula", "",
                     FLUX_NODEID_ANY, 0))) {
    flux_close(h);
    return {
        flux_plugin_res{
            .err = FLUX_PLUGIN_ERR_NOT_LOADED,
            .err_msg = strerror(errno),
        },
    };
  }
  char *formula;
  if (flux_rpc_get_unpack(f, "{s:s}", "data", formula) < 0) {
    flux_future_destroy(f);
    flux_close(h);
    return {
        flux_plugin_res{
            .err = FLUX_PLUGIN_ERR_BAD_RESPONSE,
            .err_msg = strerror(errno),
        },
    };
  }
  flux_future_destroy(f);
  flux_close(h);
  return {
      flux_plugin_res{
          .err = FLUX_PLUGIN_ERR_OK,
      },
      std::string(formula),
  };
}

flux_plugin_set_formula_res flux_plugin_set_formula(std::string formula) {
  flux_t *h;
  flux_future_t *f;
  if (!(h = flux_open(NULL, 0))) {
    return {
        flux_plugin_res{
            .err = FLUX_PLUGIN_ERR_OPEN,
            .err_msg = strerror(errno),
        },
        false,
    };
  }
  if (!(f = flux_rpc_pack(h, "job-manager.sea_priority.set_formula",
                          FLUX_NODEID_ANY, 0, "{s:s}", "data",
                          formula.c_str()))) {
    flux_close(h);
    return {
        flux_plugin_res{
            .err = FLUX_PLUGIN_ERR_NOT_LOADED,
            .err_msg = strerror(errno),
        },
        false,
    };
  }
  if (flux_future_get(f, NULL) < 0) {
    flux_future_destroy(f);
    flux_close(h);
    return {
        flux_plugin_res{
            .err = FLUX_PLUGIN_ERR_BAD_RESPONSE,
            .err_msg = strerror(errno),
        },
        false,
    };
  }
  flux_future_destroy(f);
  flux_close(h);
  return {
      flux_plugin_res{
          .err = FLUX_PLUGIN_ERR_OK,
      },
      true,
  };
}

flux_plugin_get_score_res flux_plugin_get_score(std::vector<int> user_ids) {
  flux_t *h;
  flux_future_t *f;
  if (!(h = flux_open(NULL, 0))) {
    return {
        flux_plugin_res{
            .err = FLUX_PLUGIN_ERR_OPEN,
            .err_msg = strerror(errno),
        },
    };
  }

  json_t *data = json_array();
  for (int user_id : user_ids) {
    json_array_append_new(data, json_integer(user_id));
  }
  if (!(f = flux_rpc_pack(h, "job-manager.sea_priority.get_score",
                          FLUX_NODEID_ANY, 0, "{s:o}", "data", &data))) {
    flux_close(h);
    json_decref(data);
    return {
        flux_plugin_res{
            .err = FLUX_PLUGIN_ERR_NOT_LOADED,
            .err_msg = strerror(errno),
        },
    };
  }
  json_decref(data);

  json_t *scores = json_object();
  if (flux_rpc_get_unpack(f, "{s:o}", "data", &scores) < 0) {
    flux_future_destroy(f);
    flux_close(h);
    json_decref(scores);
    return {
        flux_plugin_res{
            .err = FLUX_PLUGIN_ERR_BAD_RESPONSE,
            .err_msg = strerror(errno),
        },
    };
  }

  std::map<int, double> score_map;
  const char *user_id_key;
  json_t *score_value;
  json_object_foreach(scores, user_id_key, score_value) {
    int user_id;
    if (!parse_int(std::string(user_id_key), user_id)) {
      continue;
    }
    double score = json_real_value(score_value);
    score_map[user_id] = score;
  }
  json_decref(scores);

  flux_future_destroy(f);
  flux_close(h);
  return {
      flux_plugin_res{
          .err = FLUX_PLUGIN_ERR_OK,
      },
      score_map,
  };
}

flux_plugin_set_score_res flux_plugin_set_score(std::map<int, double> scores) {
  flux_t *h;
  flux_future_t *f;
  if (!(h = flux_open(NULL, 0))) {
    return {
        flux_plugin_res{
            .err = FLUX_PLUGIN_ERR_OPEN,
            .err_msg = strerror(errno),
        },
        false,
    };
  }
  json_t *scores_json = json_object();
  for (const auto &[user_id, score] : scores) {
    json_object_set_new(scores_json, std::to_string(user_id).c_str(),
                        json_real(score));
  }
  if (!(f = flux_rpc_pack(h, "job-manager.sea_priority.set_score",
                          FLUX_NODEID_ANY, 0, "{s:o}", "data", scores_json))) {
    flux_close(h);
    json_decref(scores_json);
    return {
        flux_plugin_res{
            .err = FLUX_PLUGIN_ERR_NOT_LOADED,
            .err_msg = strerror(errno),
        },
        false,
    };
  }
  json_decref(scores_json);
  if (flux_future_get(f, NULL) < 0) {
    flux_future_destroy(f);
    flux_close(h);
    return {
        flux_plugin_res{
            .err = FLUX_PLUGIN_ERR_BAD_RESPONSE,
            .err_msg = strerror(errno),
        },
        false,
    };
  }
  flux_future_destroy(f);
  flux_close(h);
  return {
      flux_plugin_res{
          .err = FLUX_PLUGIN_ERR_OK,
      },
      true,
  };
}

FLUX_KVS_ERR kvs_commit(flux_kvs_txn_t *txn) {
  flux_t *h;
  flux_future_t *f;
  if (!(h = flux_open(NULL, 0))) {
    return FLUX_KVS_ERR_OPEN;
  }

  if (!(f = flux_kvs_commit(h, NULL, 0, txn))) {
    flux_close(h);
    return FLUX_KVS_ERR_COMMIT;
  }

  if (flux_future_get(f, NULL) < 0) {
    flux_future_destroy(f);
    flux_close(h);
    return FLUX_KVS_ERR_GET;
  }
  flux_future_destroy(f);
  flux_close(h);
  return FLUX_KVS_ERR_SUCCESS;
}

FLUX_KVS_ERR kvs_remove(std::vector<std::string> keys) {
  flux_t *h;
  if (!(h = flux_open(NULL, 0))) {
    return FLUX_KVS_ERR_OPEN;
  }
  for (auto key : keys) {
    flux_kvs_txn_t *t;
    if (!(t = flux_kvs_txn_create())) {
      return FLUX_KVS_ERR_TXN_CREATE;
    }
    if (flux_kvs_txn_unlink(t, 0, key.c_str()) < 0) {
      return FLUX_KVS_ERR_TXN_UNLINK;
    }
    flux_future_t *f;
    if (!(f = flux_kvs_commit(h, NULL, 0, t))) {
      flux_close(h);
      return FLUX_KVS_ERR_COMMIT;
    }
    if (flux_future_get(f, NULL) < 0) {
      flux_future_destroy(f);
      flux_close(h);
      return FLUX_KVS_ERR_GET;
    }
    flux_future_destroy(f);
  }
  flux_close(h);
  return FLUX_KVS_ERR_SUCCESS;
}

FLUX_KVS_ERR kvs_remove(std::string key) { return kvs_remove({key}); }

kvs_lookups_res kvs_lookup(std::vector<std::string> keys) {
  kvs_lookups_res res;
  flux_t *h;
  if (!(h = flux_open(NULL, 0))) {
    res.err = FLUX_KVS_ERR_OPEN;
    return res;
  }

  for (auto key : keys) {
    kvs_lookup_res lookup_res;
    lookup_res.key = key;
    flux_future_t *f;
    if (!(f = flux_kvs_lookup(h, NULL, 0, key.c_str()))) {
      lookup_res.err = FLUX_KVS_ERR_LOOKUP;
      res.res.push_back(lookup_res);
      continue;
    }

    const char *value;
    if (flux_kvs_lookup_get(f, &value) < 0) {
      lookup_res.err = FLUX_KVS_ERR_LOOKUP_MISSING;
      res.res.push_back(lookup_res);
      flux_future_destroy(f);
      continue;
    }

    auto value_str = std::string(value);
    if (value_str == "") {
      lookup_res.err = FLUX_KVS_ERR_LOOKUP_MISSING;
      res.res.push_back(lookup_res);
      flux_future_destroy(f);
      continue;
    }

    lookup_res.err = FLUX_KVS_ERR_SUCCESS;
    lookup_res.value = value_str;
    res.res.push_back(lookup_res);
    flux_future_destroy(f);
  }

  flux_close(h);
  res.err = FLUX_KVS_ERR_SUCCESS;
  return res;
}

flux_list_job_unpack_res flux_list_job_unpack(json_t *value) {
  flux_list_job_unpack_res res;
  res.ok = false;

  const char *name, *cwd, *ranks, *nodelist;
  if (json_unpack(value,
                  "{s:I s:i s:i s:i s:f s:f s:f s:f s:f s:i s:s s:s s:i s:i "
                  "s:f s:i s:s s:s s:b s:b s:i s:f s:i}",
                  "id", &res.job.id, "userid", &res.job.userid, "urgency",
                  &res.job.urgency, "priority", &res.job.priority, "t_submit",
                  &res.job.t_submit, "t_depend", &res.job.t_depend, "t_run",
                  &res.job.t_run, "t_cleanup", &res.job.t_cleanup, "t_inactive",
                  &res.job.t_inactive, "state", &res.job.state, "name", &name,
                  "cwd", &cwd, "ntasks", &res.job.ntasks, "ncores",
                  &res.job.ncores, "duration", &res.job.duration, "nnodes",
                  &res.job.nnodes, "ranks", &ranks, "nodelist", &nodelist,
                  "success", &res.job.success, "exception_occurred",
                  &res.job.exception_occured, "result", &res.job.result,
                  "expiration", &res.job.expiration, "waitstatus",
                  &res.job.waitstatus) < 0) {
    res.message = "failed to unpack t_inactive";
    return res;
  }
  res.job.name = strdup(name);
  res.job.cwd = strdup(cwd);
  res.job.ranks = strdup(ranks);
  res.job.nodelist = strdup(nodelist);

  res.ok = true;
  return res;
}

flux_list_inactive_stream_res
flux_list_inactive_stream(double since, int max_entries,
                          std::function<void(json_t *)> callback) {
  flux_list_inactive_stream_res res;
  res.ok = false;
  flux_t *h;
  if (!(h = flux_open(NULL, 0))) {
    res.message = "failed to open flux handle";
    return res;
  }

  flux_future_t *f;
  json_t *c;
  int version = 0;
  if (!(c = json_pack("{s:[i]}", "states", FLUX_JOB_STATE_INACTIVE))) {
    res.message = "failed to construct constraint object";
    return res;
  }

  if (!(f = flux_rpc_pack(h, "job-list.list", FLUX_NODEID_ANY,
                          FLUX_RPC_STREAMING, "{s:i s:f s:[s] s:o}",
                          "max_entries", max_entries, "since", since, "attrs",
                          "all", "constraint", c))) {
    res.message = "failed to send job-list.list RPC";
    return res;
  }

  double t_inactive_max = since;
  while (1) {
    json_t *jobs;
    if (flux_rpc_get_unpack(f, "{s:o s?i}", "jobs", &jobs, "version",
                            &version) < 0) {
      if (errno == ENODATA) {
        break;
      }
      res.message = "failed to unpack job-list.list response";
      return res;
    }

    callback(jobs);

    /* version 0 protocol does not stream, all results in one response */
    if (version == 0) {
      break;
    }
    flux_future_reset(f);
  }
  flux_future_destroy(f);
  res.ok = true;
  return res;
}

flux_list_inactive_res flux_list_inactive(double since, int max_entries) {
  flux_list_inactive_res res;
  res.ok = false;
  double t_inactive_max = since;
  auto stream_res = flux_list_inactive_stream(
      since, max_entries, [&t_inactive_max, &res](json_t *jobs) {
        size_t index;
        json_t *value;
        json_array_foreach(jobs, index, value) {
          auto unpack_res = flux_list_job_unpack(value);
          res.jobs.push_back(unpack_res);
          if (unpack_res.ok && unpack_res.job.t_inactive > t_inactive_max) {
            t_inactive_max = unpack_res.job.t_inactive;
          }
        }
      });
  if (!stream_res.ok) {
    res.message = stream_res.message;
    return res;
  }
  res.ok = true;
  res.t_inactive_max = t_inactive_max;
  return res;
}

flux_list_res flux_list(std::vector<std::string> jobids, int max_entries) {
  flux_list_res res;
  res.ok = false;

  flux_future_t *f;
  json_t *c;
  int version = 0;
  flux_t *h;
  if (!(h = flux_open(NULL, 0))) {
    res.message = "failed to open flux handle";
    return res;
  }

  json_t *jobids_json = json_array();
  for (const auto &jobid : jobids) {
    json_array_append_new(jobids_json, json_string(jobid.c_str()));
  }

  if (!(c = json_pack("{s:o}", "ids", jobids_json))) {
    res.message = "failed to construct constraint object";
    return res;
  }

  if (!(f = flux_rpc_pack(h, "job-list.list", FLUX_NODEID_ANY,
                          FLUX_RPC_STREAMING, "{s:i s:[s] s:o}", "max_entries",
                          max_entries, "attrs", "all", "constraint", c))) {
    res.message = "failed to send job-list.list RPC";
    return res;
  }

  while (1) {
    json_t *jobs;
    if (flux_rpc_get_unpack(f, "{s:o s?i}", "jobs", &jobs, "version",
                            &version) < 0) {
      if (errno == ENODATA) {
        break;
      }
      res.message = "failed to unpack job-list.list response";
      return res;
    }

    // TODO: check if the job is completed/inactive
    // if not, skip it
    json_t *value;
    size_t index;
    json_array_foreach(jobs, index, value) {
      res.jobs.push_back(flux_list_job_unpack(value));
    }

    /* version 0 protocol does not stream, all results in one response */
    if (version == 0) {
      break;
    }
    flux_future_reset(f);
  }
  flux_future_destroy(f);
  res.ok = true;
  return res;
}
