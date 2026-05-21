// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <flux/core.h>

enum KVS_ERROR {
  KVS_SUCCESS,
  KVS_ERR_OPEN,
  KVS_ERR_PUT,
  KVS_ERR_COMMIT,
  KVS_ERR_GET,
};

KVS_ERROR kvs_commit(flux_kvs_txn_t *txn);
