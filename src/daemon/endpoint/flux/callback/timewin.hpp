// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <flux/core.h>
#include <sqlite3.h>
void ps_daemon_endpoint_flux_timewin_get(flux_t *h, const flux_msg_t *msg,
                                         sqlite3 *db);
void ps_daemon_endpoint_flux_timewin_list(flux_t *h, const flux_msg_t *msg,
                                          sqlite3 *db);
void ps_daemon_endpoint_flux_timewin_update(flux_t *h, const flux_msg_t *msg,
                                            sqlite3 *db);
void ps_daemon_endpoint_flux_timewin_create(flux_t *h, const flux_msg_t *msg,
                                            sqlite3 *db);
void ps_daemon_endpoint_flux_timewin_delete(flux_t *h, const flux_msg_t *msg,
                                            sqlite3 *db);
