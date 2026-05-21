// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "kvs.hpp"

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