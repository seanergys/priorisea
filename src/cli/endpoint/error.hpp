// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <string>

/**
 * Error raised while reaching the daemon
 */
struct ps_daemon_err_t {
  std::string err;
};

/**
 * Render a daemon error into a human readable message
 *
 * @param err Error to render
 */
std::string ps_error_message(const ps_daemon_err_t &err);
