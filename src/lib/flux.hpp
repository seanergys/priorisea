// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once

#include "error.hpp"
#include "types.hpp"
#include <flux/core.h>
#include <functional>
#include <jansson.h>
#include <string>
#include <vector>

struct ps_flux_kvs_lookup_entry_t {
  std::string key;
  ps_result_t<std::string> value;
};

struct ps_flux_job_entry_t {
  flux_jobid_t jobid;
  ps_result_t<ps_flux_job_info_t> result;
};

struct ps_flux_kvs_list_inactive_t {
  double t_inactive_max;
  std::vector<ps_result_t<ps_flux_job_info_t>> result;
};

struct ps_flux_kvs_remove_entry_t {
  std::string key;
  ps_result_t<ps_success_t> result;
};

////

ps_result_t<ps_formula_t> ps_flux_plugin_get_formula();

ps_result_t<ps_success_t>
ps_flux_plugin_set_formula(const ps_formula_t &formula);

ps_result_t<ps_score_map_t>
ps_flux_plugin_get_score(const std::vector<int> &userids);

ps_result_t<ps_success_t>
ps_flux_plugin_set_score(const ps_score_map_t &scores);

ps_result_t<ps_success_t> ps_flux_kvs_commit(flux_kvs_txn_t *txn);

ps_result_t<std::vector<ps_flux_kvs_remove_entry_t>>
ps_flux_kvs_remove(const std::vector<std::string> &keys);

ps_result_t<ps_success_t> ps_flux_kvs_remove(const std::string &key);

ps_result_t<std::vector<ps_flux_kvs_lookup_entry_t>>
ps_flux_kvs_lookup(const std::vector<std::string> &keys);

ps_result_t<std::string> ps_flux_kvs_lookup(const std::string &key);

ps_result_t<ps_flux_job_info_t> ps_flux_job_info_unpack(json_t *value);

ps_result_t<ps_success_t> ps_flux_list_inactive_jobs_stream(
    double since, int max_entries,
    std::function<ps_result_t<ps_success_t>(json_t *)> callback);

ps_result_t<ps_flux_kvs_list_inactive_t>
ps_flux_list_inactive_jobs(double since, int max_entries);

ps_result_t<std::vector<ps_flux_job_entry_t>>
ps_flux_list_jobs(const std::vector<flux_jobid_t> &jobids);

ps_result_t<std::vector<std::string>>
ps_flux_kvs_ls_dir(const std::string &key);

ps_result_t<ps_seanerbus_t> ps_flux_plugin_get_seanerbus();

ps_result_t<ps_success_t>
ps_flux_plugin_set_seanerbus(const ps_seanerbus_t &seanerbus);

////

json_t *ps_seanerbus_to_json(const ps_seanerbus_t &seanerbus);
ps_seanerbus_t ps_seanerbus_from_json(json_t *obj);

json_t *ps_flux_job_info_to_json(const ps_flux_job_info_t &info);

json_t *score_map_to_json(const ps_score_map_t &scores);
ps_score_map_t score_map_from_json(json_t *obj);
