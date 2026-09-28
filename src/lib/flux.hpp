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

/**
 * Retrieves the formula from the Flux plugin.
 *
 * @return A result containing the formula.
 */
ps_result_t<ps_formula_t> ps_flux_plugin_get_formula();

/**
 * Sets the formula in the Flux plugin.
 *
 * @param formula The formula to set.
 * @return A result indicating the success of the operation.
 */
ps_result_t<ps_success_t>
ps_flux_plugin_set_formula(const ps_formula_t &formula);

/**
 * Retrieves the score from the Flux plugin.
 *
 * @param userids The user IDs to retrieve the score for.
 * @return A result containing the score.
 */
ps_result_t<ps_score_map_t>
ps_flux_plugin_get_score(const std::vector<int> &userids);

/**
 * Sets the score in the Flux plugin.
 *
 * @param scores The scores to set.
 * @return A result indicating the success of the operation.
 */
ps_result_t<ps_success_t>
ps_flux_plugin_set_score(const ps_score_map_t &scores);

/**
 * Commits the KVS transaction in Flux.
 *
 * @param txn The KVS transaction to commit.
 * @return A result indicating the success of the operation.
 */
ps_result_t<ps_success_t> ps_flux_kvs_commit(flux_kvs_txn_t *txn);

/**
 * Removes entries from the KVS in Flux.
 *
 * @param keys The keys to remove.
 * @return A result containing the removed entries.
 */
ps_result_t<std::vector<ps_flux_kvs_remove_entry_t>>
ps_flux_kvs_remove(const std::vector<std::string> &keys);

/**
 * Removes a single entry from the KVS in Flux.
 *
 * @param key The key to remove.
 * @return A result indicating the success of the operation.
 */
ps_result_t<ps_success_t> ps_flux_kvs_remove(const std::string &key);

/**
 * Looks up entries in the KVS in Flux.
 *
 * @param keys The keys to look up.
 * @return A result containing the lookup entries.
 */
ps_result_t<std::vector<ps_flux_kvs_lookup_entry_t>>
ps_flux_kvs_lookup(const std::vector<std::string> &keys);

/**
 * Looks up a single entry in the KVS in Flux.
 *
 * @param key The key to look up.
 * @return A result containing the lookup entry.
 */
ps_result_t<std::string> ps_flux_kvs_lookup(const std::string &key);

/**
 * Unpacks a Flux job info JSON object into a `ps_flux_job_info_t` struct.
 *
 * @param value The JSON object to unpack.
 * @return A result containing the unpacked job info.
 */
ps_result_t<ps_flux_job_info_t> ps_flux_job_info_unpack(json_t *value);

/**
 * Lists inactive jobs in Flux.
 *
 * @param since The time since which to list inactive jobs.
 * @param max_entries The maximum number of entries to return.
 * @param callback The callback function to invoke for each entry.
 * @return A result indicating the success of the operation.
 */
ps_result_t<ps_success_t> ps_flux_list_inactive_jobs_stream(
    double since, int max_entries,
    std::function<ps_result_t<ps_success_t>(json_t *)> callback);

/**
 * Lists inactive jobs in Flux.
 *
 * @param since The time since which to list inactive jobs.
 * @param max_entries The maximum number of entries to return.
 * @return A result containing the list of inactive jobs.
 */
ps_result_t<ps_flux_kvs_list_inactive_t>
ps_flux_list_inactive_jobs(double since, int max_entries);

/**
 * Lists jobs in Flux.
 *
 * @param jobids The job IDs to list.
 * @return A result containing the list of jobs.
 */
ps_result_t<std::vector<ps_flux_job_entry_t>>
ps_flux_list_jobs(const std::vector<flux_jobid_t> &jobids);

/**
 * Lists entries in the KVS directory in Flux.
 *
 * @param key The key to list entries for.
 * @return A result containing the list of entries.
 */
ps_result_t<std::vector<std::string>>
ps_flux_kvs_ls_dir(const std::string &key);

/**
 * Gets the Seanerbus from the Flux plugin.
 *
 * @return A result containing the Seanerbus configuration.
 */
ps_result_t<ps_seanerbus_t> ps_flux_plugin_get_seanerbus();

/**
 * Sets the Seanerbus in the Flux plugin.
 *
 * @param seanerbus The Seanerbus configuration to set.
 * @return A result indicating the success of the operation.
 */
ps_result_t<ps_success_t>
ps_flux_plugin_set_seanerbus(const ps_seanerbus_t &seanerbus);

/**
 * Converts a Seanerbus configuration to a JSON object.
 *
 * @param seanerbus The Seanerbus configuration to convert.
 * @return A JSON object representing the Seanerbus.
 */
json_t *ps_seanerbus_to_json(const ps_seanerbus_t &seanerbus);

/**
 * Converts a JSON object to a Seanerbus configuration.
 *
 * @param obj The JSON object to convert.
 * @return A result containing the Seanerbus configuration.
 */
ps_result_t<ps_seanerbus_t> ps_seanerbus_from_json(json_t *obj);

/**
 * Converts a Flux job info to a JSON object.
 *
 * @param info The Flux job info to convert.
 * @return A JSON object representing the Flux job info.
 */
json_t *ps_flux_job_info_to_json(const ps_flux_job_info_t &info);

/**
 * Converts a score map to a JSON object.
 *
 * @param scores The score map to convert.
 * @return A JSON object representing the score map.
 */
json_t *score_map_to_json(const ps_score_map_t &scores);

/**
 * Converts a JSON object to a score map.
 *
 * @param obj The JSON object to convert.
 * @return The score map.
 */
ps_result_t<ps_score_map_t> score_map_from_json(json_t *obj);
