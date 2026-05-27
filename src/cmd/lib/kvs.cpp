// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "kvs.hpp"
#include <flux/core/kvs_txn.h>

KVS_ERR kvs_commit(flux_kvs_txn_t *txn) {
  flux_t *h;
  flux_future_t *f;
  if (!(h = flux_open(NULL, 0))) {
    return KVS_ERR_OPEN;
  }

  if (!(f = flux_kvs_commit(h, NULL, 0, txn))) {
    flux_close(h);
    return KVS_ERR_COMMIT;
  }

  if (flux_future_get(f, NULL) < 0) {
    flux_future_destroy(f);
    flux_close(h);
    return KVS_ERR_GET;
  }
  flux_future_destroy(f);
  flux_close(h);
  return KVS_ERR_SUCCESS;
}

KVS_ERR kvs_remove(std::vector<std::string> keys) {
  flux_t *h;
  if (!(h = flux_open(NULL, 0))) {
    return KVS_ERR_OPEN;
  }
  for (auto key : keys) {
    flux_kvs_txn_t *t;
    if (!(t = flux_kvs_txn_create())) {
      return KVS_ERR_TXN_CREATE;
    }
    if (flux_kvs_txn_unlink(t, 0, key.c_str()) < 0) {
      return KVS_ERR_TXN_UNLINK;
    }
    flux_future_t *f;
    if (!(f = flux_kvs_commit(h, NULL, 0, t))) {
      flux_close(h);
      return KVS_ERR_COMMIT;
    }
    if (flux_future_get(f, NULL) < 0) {
      flux_future_destroy(f);
      flux_close(h);
      return KVS_ERR_GET;
    }
    flux_future_destroy(f);
  }
  flux_close(h);
  return KVS_ERR_SUCCESS;
}

KVS_ERR kvs_remove(std::string key) { return kvs_remove({key}); }

kvs_lookups_res kvs_lookup(std::vector<std::string> keys) {
  kvs_lookups_res res;
  flux_t *h;
  if (!(h = flux_open(NULL, 0))) {
    res.err = KVS_ERR_OPEN;
    return res;
  }

  for (auto key : keys) {
    kvs_lookup_res lookup_res;
    lookup_res.key = key;
    flux_future_t *f;
    if (!(f = flux_kvs_lookup(h, NULL, 0, key.c_str()))) {
      lookup_res.err = KVS_ERR_LOOKUP;
      res.res.push_back(lookup_res);
      continue;
    }

    const char *value;
    if (flux_kvs_lookup_get(f, &value) < 0) {
      lookup_res.err = KVS_ERR_LOOKUP_MISSING;
      res.res.push_back(lookup_res);
      flux_future_destroy(f);
      continue;
    }

    auto value_str = std::string(value);
    if (value_str == "") {
      lookup_res.err = KVS_ERR_LOOKUP_MISSING;
      res.res.push_back(lookup_res);
      flux_future_destroy(f);
      continue;
    }

    lookup_res.err = KVS_ERR_SUCCESS;
    lookup_res.value = value_str;
    res.res.push_back(lookup_res);
    flux_future_destroy(f);
  }

  flux_close(h);
  res.err = KVS_ERR_SUCCESS;
  return res;
}
