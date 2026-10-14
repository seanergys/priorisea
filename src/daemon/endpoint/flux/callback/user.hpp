// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <flux/core.h>
#include <sqlite3.h>
void ps_daemon_endpoint_flux_user_get(flux_t *h, const flux_msg_t *msg,
                                      sqlite3 *db);
void ps_daemon_endpoint_flux_user_list(flux_t *h, const flux_msg_t *msg,
                                       sqlite3 *db);
void ps_daemon_endpoint_flux_user_update(flux_t *h, const flux_msg_t *msg,
                                         sqlite3 *db);
