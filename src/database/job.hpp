// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <flux/core.h>
#include <map>

/**
 * Map of job id to its get result, one entry per requested id
 */
using ps_db_sqlite_jobs_get_t = std::map<flux_jobid_t, ps_result_t<ps_job_t>>;
