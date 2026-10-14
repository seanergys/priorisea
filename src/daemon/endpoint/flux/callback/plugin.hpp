// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <flux/core.h>
#include <sqlite3.h>

void ps_daemon_endpoint_flux_plugin_sync(flux_t *h, const flux_msg_t *msg,
                                         sqlite3 *db);

void ps_daemon_endpoint_flux_plugin_debug(flux_t *h, const flux_msg_t *msg,
                                          sqlite3 *db);
