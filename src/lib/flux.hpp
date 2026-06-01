// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <flux/core.h>
#include <functional>
#include <jansson.h>
#include <map>
#include <string>
#include <vector>

////

enum FLUX_PLUGIN_ERR {
  FLUX_PLUGIN_ERR_OK,
  FLUX_PLUGIN_ERR_OPEN,
  FLUX_PLUGIN_ERR_NOT_LOADED,
  FLUX_PLUGIN_ERR_BAD_RESPONSE,
};

struct flux_plugin_res {
  FLUX_PLUGIN_ERR err;
  std::string err_msg;
};

enum FLUX_KVS_ERR {
  FLUX_KVS_ERR_SUCCESS,
  FLUX_KVS_ERR_OPEN,
  FLUX_KVS_ERR_PUT,
  FLUX_KVS_ERR_COMMIT,
  FLUX_KVS_ERR_GET,
  FLUX_KVS_ERR_TXN_CREATE,
  FLUX_KVS_ERR_TXN_UNLINK,
  FLUX_KVS_ERR_LOOKUP,
  FLUX_KVS_ERR_LOOKUP_MISSING,
};

struct flux_list_job_res {
  long long id;
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

////

struct flux_plugin_get_formula_res : flux_plugin_res {
  std::string payload;
};
flux_plugin_get_formula_res flux_plugin_get_formula();

struct flux_plugin_set_formula_res : flux_plugin_res {
  bool formula_set;
};
flux_plugin_set_formula_res flux_plugin_set_formula(std::string payload);

struct flux_plugin_get_score_res : flux_plugin_res {
  std::map<int, double> scores;
};
flux_plugin_get_score_res flux_plugin_get_score(std::vector<int> user_ids);

struct flux_plugin_set_score_res : flux_plugin_res {
  bool score_set;
};
flux_plugin_set_score_res flux_plugin_set_score(std::map<int, double> scores);

FLUX_KVS_ERR kvs_commit(flux_kvs_txn_t *txn);

FLUX_KVS_ERR kvs_remove(std::string key);

FLUX_KVS_ERR kvs_remove(std::vector<std::string> keys);

struct kvs_lookup_res {
  FLUX_KVS_ERR err;
  std::string key;
  std::string value;
};
struct kvs_lookups_res {
  FLUX_KVS_ERR err;
  std::vector<kvs_lookup_res> res;
};
kvs_lookups_res kvs_lookup(std::vector<std::string> keys);

struct flux_list_job_unpack_res {
  bool ok;
  std::string message;
  flux_list_job_res job;
};
flux_list_job_unpack_res flux_list_job_unpack(json_t *value);

struct flux_list_inactive_stream_res {
  bool ok;
  std::string message;
};
flux_list_inactive_stream_res
flux_list_inactive_stream(double since, int max_entries,
                          std::function<void(json_t *)> callback);

struct flux_list_inactive_res {
  bool ok;
  std::string message;
  double t_inactive_max;
  std::vector<flux_list_job_unpack_res> jobs;
};
flux_list_inactive_res flux_list_inactive(double since, int max_entries);

struct flux_list_res {
  bool ok;
  std::string message;
  std::vector<flux_list_job_unpack_res> jobs;
};
flux_list_res flux_list(std::vector<std::string> jobids);

struct flux_kvs_ls_dir_res {
  bool ok;
  std::string message;
  std::vector<std::string> keys;
};
flux_kvs_ls_dir_res flux_kvs_ls_dir(std::string dir);
