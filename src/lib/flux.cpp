// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "flux.hpp"
#include "util.hpp"
#include <flux/core.h>
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

  std::map<int, int> score_map;
  const char *user_id_key;
  json_t *score_value;
  json_object_foreach(scores, user_id_key, score_value) {
    int user_id;
    if (!parse_int(std::string(user_id_key), user_id)) {
      continue;
    }
    int score = json_integer_value(score_value);
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

flux_plugin_set_score_res flux_plugin_set_score(std::map<int, int> scores) {
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
                        json_integer(score));
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
