// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/daemon/endpoint/flux/endpoint.hpp"

int ps_daemon_endpoint_flux_decay_get(
    const ps_daemon_endpoint_flux_callback_args_t &args);
int ps_daemon_endpoint_flux_decay_update(
    const ps_daemon_endpoint_flux_callback_args_t &args);
int ps_daemon_endpoint_flux_decay_reset(
    const ps_daemon_endpoint_flux_callback_args_t &args);
