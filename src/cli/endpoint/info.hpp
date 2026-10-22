#pragma once

#include "src/lib/types.hpp"

/**
 * Daemon information returned by the info endpoint
 */
struct ps_cli_endpoint_info_get_t {
  int version;
  ps_db_info_t info;
};
