// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <flux/core.h>
#include <string>
#include <vector>

ps_result_t<ps_job_t> ps_cli_endpoint_flux_job_get(const std::string &jobid,
                                                   const std::string &format);

ps_result_t<std::vector<ps_job_t>>
ps_cli_endpoint_flux_job_list(const std::vector<std::string> &jobids,
                              const std::string &format, int limit);

ps_result_t<ps_success_t>
ps_cli_endpoint_flux_job_update(const std::vector<std::string> &jobids,
                                bool _new);
