#pragma once
#include "src/lib/error.hpp"
#include <flux/core.h>

/**
 * Extract the error kind from a flux future
 *
 * @param f Future to inspect
 */
ps_error_kind_t ps_cli_endpoint_unpack_err(flux_future_t *f);
