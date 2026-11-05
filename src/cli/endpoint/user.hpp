#pragma once
#include "src/lib/types.hpp"
#include <vector>

/**
 * User data along the jobs used to compute its user score
 */
struct ps_cli_endpoint_user_get_t {
  ps_user_t user;
  std::vector<ps_job_t> jobs;
};
