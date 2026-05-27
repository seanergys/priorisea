// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <flux/core.h>
#include <string>
#include <vector>

enum KVS_ERR {
  KVS_ERR_SUCCESS,
  KVS_ERR_OPEN,
  KVS_ERR_PUT,
  KVS_ERR_COMMIT,
  KVS_ERR_GET,
  KVS_ERR_TXN_CREATE,
  KVS_ERR_TXN_UNLINK,
  KVS_ERR_LOOKUP,
  KVS_ERR_LOOKUP_MISSING,
};
struct kvs_lookup_res {
  KVS_ERR err;
  std::string key;
  std::string value;
};
struct kvs_lookups_res {
  KVS_ERR err;
  std::vector<kvs_lookup_res> res;
};

KVS_ERR kvs_commit(flux_kvs_txn_t *txn);

KVS_ERR kvs_remove(std::string key);

KVS_ERR kvs_remove(std::vector<std::string> keys);

kvs_lookups_res kvs_lookup(std::vector<std::string> keys);
