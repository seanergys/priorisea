// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <flux/core.h>
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
  std::map<int, int> scores;
};
flux_plugin_get_score_res flux_plugin_get_score(std::vector<int> user_ids);

struct flux_plugin_set_score_res : flux_plugin_res {
  bool score_set;
};
flux_plugin_set_score_res flux_plugin_set_score(std::map<int, int> scores);

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
